// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "FS_PlayerController.generated.h"

class UFS_PlayerHUDWidget;
class UAbilitySystemComponent;
class UFS_AttributeSet_Stats_;
class UInputMappingContext;
class UInputAction;
class UGameplayAbility;

/**
 * Spawns and initializes the HUD for the local player.
 * Waits until PlayerState + ASC are valid, then wires everything up.
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFS_PlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_PrimaryAttack;

	// Blueprint exposed widget class to spawn (set this in the BP_PlayerController)
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UFS_PlayerHUDWidget> HUDWidgetClass;

	// The spawned widget instance
	UPROPERTY(Transient)
	TObjectPtr<UFS_PlayerHUDWidget> HUDWidget = nullptr;

	// Re-attempt timer to initialize HUD after possession/replication
	FTimerHandle HUDInitRetryTimer;

	// Try to create the widget and bind it to ASC/Stats; Returns true on success
	bool TryInitializeHUD();

	// Wrapper called by the timer (SetTimer requires a void, no-arg function)
	// Calls TryInitializeHUD and clears the timer on success
	UFUNCTION() // UFUNCTION is optional here, but harmless and tidy
		void TryInitializeHUD_Tick();

	void OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& Reason);

	// Input Handlers
	void Input_PrimaryAttack();
};