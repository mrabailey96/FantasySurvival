// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "FS_EnemyHealthBarWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;

/**
 * Logic-only health bar widget. Layout (ProgressBar) is provided by a UMG asset
 * that subclasses this class and exposes a ProgressBar named "PB_Health".
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_EnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Call once (from the enemy) after the widget is created
	UFUNCTION(BlueprintCallable, Category = "FS|GAS")
	void InitializeFromASC(UAbilitySystemComponent* InASC);

protected:
	// UUserWidget
	virtual void NativeDestruct() override;

private:
	// Progress bar bound from the UMG Designer. Name MUST be PB_Health
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_Health = nullptr;

	// The top level container in the designer (Sizebox/Border/Overlay named RootContainer)
	UPROPERTY(meta = (BindWidgetOption))
	TObjectPtr<UWidget> RootContainer = nullptr;

	// Weak ref so GC is safe
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	// Cached attributes for quick access
	FGameplayAttribute HealthAttr;
	FGameplayAttribute MaxHealthAttr;

	// Delegate handle for unbinding
	FDelegateHandle HealthChangedHandle;

	// Hide Health Bar when Health is full
	UPROPERTY(EditAnywhere, Category = "FS|Polish")
	bool bHideWhenFull = true;

	// Delegate callback from GAS when Health changes
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Reads MaxHealth and updates PB_Health percent from a given Health value
	void RefreshPercentFrom(float CurrentHealth);

	// Apply hide when full
	void ApplyPolish(float Percent);
};
