// Fill out your copyright notice in the Description page of Project Settings.


#include "System/FS_PlayerController.h"
#include "UI/FS_PlayerHUDWidget.h"
#include "AbilitySystem/FS_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameplayTagContainer.h"

AFS_PlayerController::AFS_PlayerController()
{
	// Reasonable net update rate for snappy UI (client-side)
	SetNetUpdateFrequency(60.0f);
}

void AFS_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		// Add the mapping context so our IA can fire
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsys->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	// Try now; if not ready (PS/ASC not yet replicated), retry shortly
	if (!TryInitializeHUD())
	{
		GetWorldTimerManager().SetTimer(HUDInitRetryTimer, this, &AFS_PlayerController::TryInitializeHUD_Tick, 0.25f, true);
	}
}

void AFS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_PrimaryAttack)
		{
			// Trigger on Pressed (Started) - Feel free to also bind Completed/Cancelled for charge attacks later
			EIC->BindAction(IA_PrimaryAttack, ETriggerEvent::Started, this, &AFS_PlayerController::Input_PrimaryAttack);
		}
	}
}

void AFS_PlayerController::Input_PrimaryAttack()
{
	// Activate any abilities tagged with Ability.Melee.Light
	static const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Melee.Light"));

	if (AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			FGameplayTagContainer LocalTags;
			LocalTags.AddTag(AttackTag);
			ASC->TryActivateAbilitiesByTag(LocalTags);
		}
	}
}

void AFS_PlayerController::TryInitializeHUD_Tick()
{
	// Call the bool initializer; clear the timer once it has succeeded
	if (TryInitializeHUD())
	{
		GetWorldTimerManager().ClearTimer(HUDInitRetryTimer);
	}
}

bool AFS_PlayerController::TryInitializeHUD()
{
	// We need a valid PlayerState that is our GAS owner
	AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>();
	if (!PS)
	{
		return false; // Not ready yet
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	UFS_AttributeSet_Stats* Stats = PS->GetStats();
	if (!ASC || !Stats)
	{
		return false; // Still not ready
	}

	PS->GetAbilitySystemComponent()->AbilityFailedCallbacks.AddUObject(this, &AFS_PlayerController::OnAbilityFailed);

	// Create the Widget once
	if (!HUDWidget)
	{
		if (!HUDWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("HUDWidgetClass not set on PlayerController. Assign in BP class"));
			return true; // Avoid infinite retry loop
		}

		HUDWidget = CreateWidget<UFS_PlayerHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport(0);
		}
	}

	// Initialize with ASC + Attributes (Safe to call multiple times)
	if (HUDWidget)
	{
		HUDWidget->InitializeFromASC(ASC, Stats);
	}

	// Success: stop retrying
	if (HUDInitRetryTimer.IsValid())
	{
		GetWorldTimerManager().ClearTimer(HUDInitRetryTimer);
	}
	return true;
}

void AFS_PlayerController::OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& Reason)
{
	if (Reason.HasTag(FGameplayTag::RequestGameplayTag(FName("Ability.Fail.Stamina"))))
	{
		// Show a message
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Not enough Stamina!"));
	}
}