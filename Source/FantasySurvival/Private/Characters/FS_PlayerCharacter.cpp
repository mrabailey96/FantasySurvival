// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FS_PlayerCharacter.h"
#include "AbilitySystem/FS_AS_PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

// Sets default values
AFS_PlayerCharacter::AFS_PlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// -------- Camera rig --------
	// Classic Third Person Feel
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 325.0f;
	SpringArmComponent->SocketOffset = FVector(0.0f, 50.0f, 70.0f); // slight over-shoulder
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bDoCollisionTest = true;
	SpringArmComponent->bEnableCameraLag = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false; // boom handls rotation

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// -------- Character Movement --------
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	// -------- Gameplay Ability System --------
	// Character doesn't own the ASC; PlayerState does
	AbilitySystemComponent = nullptr;
	StatsSet = nullptr;
}

// Called when the game starts or when spawned
void AFS_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Debug (Comment out later): Print stats on owning client after OnRep_PlayerState
	if (IsLocallyControlled())
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (const AFS_AS_PlayerState* PlayerState = GetPlayerState<AFS_AS_PlayerState>())
				{
					UE_LOG(LogTemp, Log, TEXT("Stats -> Health: %.1f/%.1f, Mana: %.1f/%.1f, Stamina: %.1f/%.1f"),
						PlayerState->GetHealth(), PlayerState->GetMaxHealth(),
						PlayerState->GetMana(), PlayerState->GetMaxMana(),
						PlayerState->GetStamina(), PlayerState->GetMaxStamina())
				}
			}, 0.2f, false);
	}
}

// Called to bind functionality to input
void AFS_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Ensure out mapping context is active for the local player
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		AddInputContext(PlayerController);
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement
		if (IA_Move)
		{
			EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFS_PlayerCharacter::Move);
		}

		// Look
		if (IA_Look)
		{
			EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFS_PlayerCharacter::Look);
		}

		// Jump
		if (IA_Jump)
		{
			// Using ACharacter's built-in Jump/StopJumping functions
			EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
			EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);
		}
	}
	
}

void AFS_PlayerCharacter::AddInputContext(APlayerController* PC)
{
	if (!IMC || !PC) return;

	if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(IMC, 0);
		}
	}
}

UAbilitySystemComponent* AFS_PlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AFS_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// SERVER: Initialize our ASC ActorInfo now that we have a PlayerState and Controller
	InitializeAbilitySystem();

	// SERVER: Apply initial attributes (Health/Mana/Stamina)
	ApplyDefaultAttributes();
}

void AFS_PlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// CLIENT (Owning): Re-initialize ASC ActorInfo when PlayerState replicates
	InitializeAbilitySystem();
}

void AFS_PlayerCharacter::InitializeAbilitySystem()
{
	// Ensure we have a PlayerState and it implements our GAS Owner
	AFS_AS_PlayerState* PS = GetPlayerState<AFS_AS_PlayerState>();
	if (!PS) return;

	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	StatsSet = PS->GetStats();

	if (AbilitySystemComponent)
	{
		// Bind the ASC to this pawn (Avatar) and our PlayerState (Owner)
		// This is required so abilities can reference "ActorInfo" (Owner Avatar, Owner Actor, etc)
		AbilitySystemComponent->InitAbilityActorInfo(PS, this); // Owner, Avatar

		// Optionally, if we plan to use InputID-based abilities later, we can set input bindings here
		// e.g., AbilitySystemComponent->SetTagMapCount(SomeTag, 1);
	}
}

void AFS_PlayerCharacter::ApplyDefaultAttributes()
{
	// Only the server should APPLY GameplayEffects that modify attributes at spawn
	if (!HasAuthority() || !AbilitySystemComponent || !DefaultInitializeAttributesEffect) return;

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultInitializeAttributesEffect, /* Level */ 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	// After this, Health/Mana/Stamina values are set as defined by the GE
}

void AFS_PlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>(); // X = Right, Y = Forward
	if (!Controller || Axis.IsNearlyZero()) return;

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void AFS_PlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>(); // X = Yaw, Y = Pitch
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}