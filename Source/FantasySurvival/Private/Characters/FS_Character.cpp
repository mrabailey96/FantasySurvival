// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FS_Character.h"
#include "Player/FS_PlayerState.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "UI/FS_PauseMenuWidget.h"
#include "Game/FS_GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AFS_Character::AFS_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// Classic third-person feel
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	// --- Camera rig ---
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 325.f;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f); // slight over-shoulder
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.f;
	CameraBoom->CameraLagMaxDistance = 60.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // boom handles rotation
}

UAbilitySystemComponent* AFS_Character::GetAbilitySystemComponent() const
{
	return ASC;
}

void AFS_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>())
	{
		ASC = Cast<UFS_AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		if (ASC)
		{
			ASC->InitAbilityActorInfo(PS, this);

			if (HasAuthority())
			{
				// NEW: copy the class picked in the main menu
				if (UFS_GameInstance* GI = GetWorld()->GetGameInstance<UFS_GameInstance>())
				{
					PS->SelectedClass = GI->GetPendingClass();
				}

				// Apply the correct per-class init now
				PS->ApplyClassInitialization();
			}
		}
	}
}

void AFS_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>())
	{
		ASC = Cast<UFS_AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		if (ASC)
		{
			// Re-establish actor info on clients after PlayerState replicates
			ASC->InitAbilityActorInfo(PS, this);

			// DO NOT apply init effects here; server already did it in PossessedBy
			// (Clients will see replicated attributes after the GE is applied server-side)
		}
	}
}

void AFS_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Ensure our mapping context is active for the local player
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		AddInputContext(PC);
	}

	// Bind actions using EnhancedInputComponent
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		if (IA_Move)
			EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFS_Character::Move);

		// Look
		if (IA_Look)
			EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFS_Character::Look);

		// Jump
		if (IA_Jump)
		{
			EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
			EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);
		}

		if (IA_PrimaryAttack)
		{
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Started, this, &AFS_Character::PrimaryAttack_Pressed);
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Completed, this, &AFS_Character::PrimaryAttack_Released);
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Canceled, this, &AFS_Character::PrimaryAttack_Released);
		}

		// Ability 1
		if (IA_Ability1)
		{
			EIC->BindAction(IA_Ability1, ETriggerEvent::Started, this, &AFS_Character::Ability1_Pressed);
			EIC->BindAction(IA_Ability1, ETriggerEvent::Completed, this, &AFS_Character::Ability1_Released);
			EIC->BindAction(IA_Ability1, ETriggerEvent::Canceled, this, &AFS_Character::Ability1_Released);
		}

		// Pause
		if (IA_Pause)
		{
			EIC->BindAction(IA_Pause, ETriggerEvent::Started, this, &AFS_Character::TogglePauseMenu);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FS_Character expects UEnhancedInputComponent on the pawn."));
	}
}

void AFS_Character::AddInputContext(APlayerController* PC)
{
	if (!IMC || !PC) return;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(IMC, /*Priority*/0);
		}
	}
}

void AFS_Character::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>(); // X=Right, Y=Forward
	if (!Controller || Axis.IsNearlyZero()) return;

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void AFS_Character::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>(); // X=Yaw, Y=Pitch
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AFS_Character::PrimaryAttack_Pressed()
{
	if (ASC) ASC->InputPressed(EFSAbilityInputID::PrimaryAttack);
}

void AFS_Character::PrimaryAttack_Released()
{
	if (ASC) ASC->InputReleased(EFSAbilityInputID::PrimaryAttack);
}

void AFS_Character::Ability1_Pressed()
{
	if (ASC) ASC->InputPressed(EFSAbilityInputID::Ability1);
}

void AFS_Character::Ability1_Released()
{
	if (ASC) ASC->InputReleased(EFSAbilityInputID::Ability1);
}

void AFS_Character::TogglePauseMenu()
{
	const bool bIsPaused = GetWorld()->IsPaused();
	if (!bIsPaused) ShowPauseMenu();
	else            HidePauseMenu();
}

void AFS_Character::ShowPauseMenu()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC || !PauseMenuClass) return;

	// Create once
	if (!PauseMenuInstance)
	{
		PauseMenuInstance = CreateWidget<UFS_PauseMenuWidget>(PC, PauseMenuClass);
		if (!PauseMenuInstance) return;

		// Bind widget signals
		PauseMenuInstance->OnResumeRequested.AddDynamic(this, &AFS_Character::HandlePauseResume);
		PauseMenuInstance->OnQuitToMainRequested.AddDynamic(this, &AFS_Character::HandlePauseQuitToMain);
		PauseMenuInstance->OnQuitToDesktopRequested.AddDynamic(this, &AFS_Character::HandlePauseQuitToDesktop);
		// SettingsRequested currently does nothing
	}

	// Add to viewport, focus it, switch to UI input, show cursor
	PauseMenuInstance->AddToViewport(100);

	FInputModeUIOnly UI;
	UI.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
	UI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(UI);
	PC->SetShowMouseCursor(true);

	// Pause
	PC->SetPause(true);
	// Optional: also block pawn input explicitly
	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);
}

void AFS_Character::HidePauseMenu()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	// Unpause
	PC->SetPause(false);

	// Remove widget
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->RemoveFromParent();
	}

	// Restore input to game
	FInputModeGameOnly GameOnly;
	PC->SetInputMode(GameOnly);
	PC->SetShowMouseCursor(false);
	PC->SetIgnoreLookInput(false);
	PC->SetIgnoreMoveInput(false);
}

void AFS_Character::HandlePauseResume()
{
	HidePauseMenu();
}

void AFS_Character::HandlePauseQuitToMain()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) return;

	// Clear selections so player can pick again
	if (UFS_GameInstance* GI = GetWorld()->GetGameInstance<UFS_GameInstance>())
	{
		GI->ClearPending();
	}

	// Cleanly unpause before travel
	HidePauseMenu();

	// Load your main menu map (replace with your exact map name)
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void AFS_Character::HandlePauseQuitToDesktop()
{
	APlayerController* PC = Cast<APlayerController>(Controller);
	HidePauseMenu(); // just to be tidy
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, /*bIgnorePlatformRestrictions=*/false);
}