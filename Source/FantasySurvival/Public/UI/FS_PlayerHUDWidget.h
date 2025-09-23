// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "GameplayEffectTypes.h"
#include "FS_PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UFS_AttributeSet_Stats;
class UAbilitySystemComponent;
class AFS_PlayerState;

/**
 * GAS-driven HUD:
 * - Health / Mana / Stamina bars (+ text)
 * - Cooldown bars for abilities via tags
 * - State icons for Blocking / Attacking
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_PlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void InitializeFromPlayerState(AFS_PlayerState* InPlayerState);

protected:
    virtual void NativeDestruct() override;

    // ----- BindWidget targets (create these in your BP and name exactly) -----
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* HealthBar = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* ManaBar = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* StaminaBar = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* HealthText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* ManaText = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* StaminaText = nullptr;

    // Ability cooldown bars (overlay style bars are fine)
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* PrimaryAttackCooldown = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UProgressBar* BlockCooldown = nullptr; // optional, if you add a block cooldown later

    // State icons (optional/testing)
    UPROPERTY(meta = (BindWidgetOptional)) UImage* BlockingIcon = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UImage* AttackingIcon = nullptr;

    // ----- GAS references -----
    UPROPERTY(Transient) TWeakObjectPtr<UAbilitySystemComponent> ASC;
    UPROPERTY(Transient) TWeakObjectPtr<UFS_AttributeSet_Stats> Stats;

    // ----- Attribute bindings -----
    void BindAttributeDelegates();
    void RefreshAll();

    void OnHealthChanged(const struct FOnAttributeChangeData& Data);
    void OnMaxHealthChanged(const struct FOnAttributeChangeData& Data);
    void OnManaChanged(const struct FOnAttributeChangeData& Data);
    void OnMaxManaChanged(const struct FOnAttributeChangeData& Data);
    void OnStaminaChanged(const struct FOnAttributeChangeData& Data);
    void OnMaxStaminaChanged(const struct FOnAttributeChangeData& Data);

    // ----- Cooldowns / tags -----
    void StartCooldownTicker();
    void StopCooldownTicker();
    void TickCooldowns();

    FTimerHandle CooldownTimer;

    void BindStateTagEvents();
    void OnStateTagChanged(FGameplayTag Tag, int32 NewCount);

private:
    float SafeDiv(float A, float B) const { return (B <= KINDA_SMALL_NUMBER) ? 0.f : (A / B); }
};
