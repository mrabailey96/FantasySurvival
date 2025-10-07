// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FS_PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
class UFS_AttributeSet_Stats;

struct FOnAttributeChangeData;

/**
 * UFS_PlayerHUDWidget
 * - Logic widget for showing Health / Mana / Stamina
 * - Receives references to ASC + AttributeSet
 * - Subscribes to attribute change delegates and updates bars live
 *
 * Create a UMG BP that inherits from this class and place ProgressBars named:
 *   HealthBar, ManaBar, StaminaBar (optional Text: HealthText, ManaText, StaminaText)
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_PlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//Wire up ASC + Attributes. Safe to call once ASC is initialized (after possession/onrep_)
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeFromASC(UAbilitySystemComponent* InASC, UFS_AttributeSet_Stats* InStats);

protected:
	// UMG Bindings: Assign via designer by matching names (BindWidget)
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UProgressBar* HealthBar = nullptr;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UProgressBar* ManaBar = nullptr;

	UPROPERTY(meta = (BindWidget, AllowPrivateAcess = "true"))
	UProgressBar* StaminaBar = nullptr;

	// Optional text readouts (safe if we dont use them or they dont exist) - mainly for testing
	UPROPERTY(meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	class UTextBlock* HealthText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	class UTextBlock* ManaText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	class UTextBlock* StaminaText = nullptr;

	// Cached pointers (not owned)
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> ASC = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UFS_AttributeSet_Stats> Stats = nullptr;

	// Delegate handles so we could unbind later if needed
	FDelegateHandle OnHealthChangedHandle;
	FDelegateHandle OnMaxHealthChangedHandle;
	FDelegateHandle OnManaChangedHandle;
	FDelegateHandle OnMaxManaChangedHandle;
	FDelegateHandle OnStaminaChangedHandle;
	FDelegateHandle OnMaxStaminaChangedHandle;

	// Wrapper Handlers that match the delegate signature
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleManaChanged(const FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);

	// Apply current values to the UI
	void RefreshAll();

	// Individual update helpers
	void UpdateHealth();
	void UpdateMana();
	void UpdateStamina();

	// Utility: Set a bar safely with Current/Max (0..1) + optional text
	void SetBarAndText(UProgressBar* Bar, UTextBlock* Text, float Current, float Max);

	// Unbind on destruction to be extra safe
	virtual void NativeDestruct() override;
};