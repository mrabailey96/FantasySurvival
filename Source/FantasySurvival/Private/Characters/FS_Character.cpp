// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FS_Character.h"
#include "Player/FS_PlayerState.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "AbilitySystem/FS_ClassConfig.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

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
				if (UFS_GameInstance* GI = GetWorld()->GetGameInstance<UFS_GameInstance>())
				{
					PS->SelectedClass = GI->GetPendingClass();
				}

				ApplyClassAppearance();
				ApplyClassWeapons();

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

		ApplyClassAppearance();
		ApplyClassWeapons();
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

		// Primary Attack
		if (IA_PrimaryAttack)
		{
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Started, this, &AFS_Character::PrimaryAttack_Pressed);
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Completed, this, &AFS_Character::PrimaryAttack_Released);
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Canceled, this, &AFS_Character::PrimaryAttack_Released);
		}

		// Block
		if (IA_Block)
		{
			EIC->BindAction(IA_Block, ETriggerEvent::Started, this, &AFS_Character::Block_Pressed);
			EIC->BindAction(IA_Block, ETriggerEvent::Completed, this, &AFS_Character::Block_Released);
			EIC->BindAction(IA_Block, ETriggerEvent::Canceled, this, &AFS_Character::Block_Released);
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

void AFS_Character::ApplyClassAppearance()
{
	AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>();
	if (!PS || !PS->ClassConfig) return;

	const EFSPlayerClass Class = PS->SelectedClass;

	const FFS_ClassAppearance* Appearance = PS->ClassConfig->ClassAppearance.Find(Class);
	if (!Appearance) return;

	// Load mesh if it is a soft reference
	USkeletalMesh* NewMesh = Appearance->Mesh.IsNull() ? nullptr : Appearance->Mesh.LoadSynchronous();
	if (NewMesh)
	{
		GetMesh()->SetSkeletalMesh(NewMesh);
	}

	if (Appearance->AnimClass)
	{
		GetMesh()->SetAnimInstanceClass(Appearance->AnimClass);
	}

	// Materials
	for (int32 i = 0; i < Appearance->Materials.Num(); ++i)
	{
		if (Appearance->Materials[i])
		{
			GetMesh()->SetMaterial(i, Appearance->Materials[i]);
		}
	}
}

UMeshComponent* AFS_Character::CreateAndAttachWeapon(const FFS_WeaponSpec& Spec, FName DebugName)
{
	if (!GetMesh()) return nullptr;

	// Resolve which mesh type we’re using
	USkeletalMesh* SK = Spec.SkeletalMesh.IsNull() ? nullptr : Spec.SkeletalMesh.LoadSynchronous();
	UStaticMesh* SM = (SK == nullptr && !Spec.StaticMesh.IsNull()) ? Spec.StaticMesh.LoadSynchronous() : nullptr;
	if (!SK && !SM) return nullptr;

	// Create an appropriate component
	UMeshComponent* NewComp = nullptr;

	if (SK)
	{
		USkeletalMeshComponent* SKC = NewObject<USkeletalMeshComponent>(this, DebugName);
		SKC->SetSkeletalMesh(SK);
		if (Spec.SkeletalAnimClass)
		{
			SKC->SetAnimInstanceClass(Spec.SkeletalAnimClass);
		}
		NewComp = SKC;
	}
	else if (SM)
	{
		UStaticMeshComponent* SMC = NewObject<UStaticMeshComponent>(this, DebugName);
		SMC->SetStaticMesh(SM);
		NewComp = SMC;
	}

	if (!NewComp) return nullptr;

	NewComp->SetMobility(EComponentMobility::Movable);
	NewComp->SetIsReplicated(true); // basic replication; for heavy MP you may prefer an attached Actor

	// Attach to the character mesh at the desired socket (fallback to class defaults if none)
	const FName Socket = (Spec.AttachSocket.IsNone() ? FName(TEXT("weapon_r_socket")) : Spec.AttachSocket);
	NewComp->SetupAttachment(GetMesh(), Socket);

	// Register and apply transform/materials
	NewComp->RegisterComponent();
	NewComp->SetRelativeLocationAndRotation(Spec.RelativeLocation, Spec.RelativeRotation);
	NewComp->SetRelativeScale3D(Spec.RelativeScale);

	for (int32 i = 0; i < Spec.Materials.Num(); ++i)
	{
		if (Spec.Materials[i])
		{
			// Both Static/Skeletal derive from UMeshComponent → SetMaterial available
			NewComp->SetMaterial(i, Spec.Materials[i]);
		}
	}

	return NewComp;
}

void AFS_Character::ApplyClassWeapons()
{
	AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>();
	if (!PS || !PS->ClassConfig) return;

	// Destroy/clear any existing components first
	auto DestroyIfValid = [](TObjectPtr<UMeshComponent>& Comp)
		{
			if (Comp)
			{
				Comp->DestroyComponent();
				Comp = nullptr;
			}
		};

	DestroyIfValid(RightWeaponComp);
	DestroyIfValid(LeftWeaponComp);

	const EFSPlayerClass Class = PS->SelectedClass;

	const FFS_ClassWeapons* ClassWeaps = PS->ClassConfig->ClassWeapons.Find(Class);
	if (!ClassWeaps) return;

	// Right hand (default socket fallback is weapon_r_socket)
	if (!ClassWeaps->RightHand.SkeletalMesh.IsNull() || !ClassWeaps->RightHand.StaticMesh.IsNull())
	{
		RightWeaponComp = CreateAndAttachWeapon(ClassWeaps->RightHand, TEXT("RightWeaponComp"));
	}

	// Left hand (default socket fallback is weapon_l_socket)
	FFS_WeaponSpec LeftSpec = ClassWeaps->LeftHand;
	if (LeftSpec.AttachSocket.IsNone())
	{
		LeftSpec.AttachSocket = FName(TEXT("weapon_l_socket"));
	}

	if (!LeftSpec.SkeletalMesh.IsNull() || !LeftSpec.StaticMesh.IsNull())
	{
		LeftWeaponComp = CreateAndAttachWeapon(LeftSpec, TEXT("LeftWeaponComp"));
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

void AFS_Character::Block_Pressed()
{
	if (ASC) ASC->InputPressed(EFSAbilityInputID::Block);
}

void AFS_Character::Block_Released()
{
	if (ASC) ASC->InputReleased(EFSAbilityInputID::Block);
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