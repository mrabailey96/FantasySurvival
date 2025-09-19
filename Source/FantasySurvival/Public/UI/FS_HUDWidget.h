// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayEffectTypes.h"
#include "FS_HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/**
 * Lightweight in-game HUD (CommonUI) that listens to ASC attribute updates.
 * Bind your UMG children in WBP_HUD derived from this class.
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_HUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	// Call once on create/activate with your pawn's ASC and attribute getters.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeForASC(UAbilitySystemComponent* InASC, FGameplayAttribute InHealthAttr, FGameplayAttribute InMaxHealthAttr,
		FGameplayAttribute InStaminaAttr, FGameplayAttribute InMaxStaminaAttr);

	// Optional Blueprint-callable setters if you want to drive purely from BP.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetStamina(float Current, float Max);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	// ——— UMG Bindings (create these in the BP and bind) ———
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* HealthBar = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* HealthText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* StaminaBar = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StaminaText = nullptr;

private:
	// Cached ASC + attributes
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC = nullptr;

	FGameplayAttribute HealthAttr;
	FGameplayAttribute MaxHealthAttr;
	FGameplayAttribute StaminaAttr;
	FGameplayAttribute MaxStaminaAttr;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle StaminaChangedHandle;
	FDelegateHandle MaxStaminaChangedHandle;

	void SubscribeAttributeDelegates();
	void UnsubscribeAttributeDelegates();

	// Pulls latest values and updates the UI
	void RefreshBars();

	// Delegate callbacks
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);

};
