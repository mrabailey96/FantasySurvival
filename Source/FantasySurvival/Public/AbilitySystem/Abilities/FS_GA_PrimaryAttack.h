// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "FS_GA_PrimaryAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UFS_CharacterAnimInstance;

UCLASS()
class FANTASYSURVIVAL_API UFS_GA_PrimaryAttack : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UFS_GA_PrimaryAttack();

    // Optional: cooldown / damage effect you already use
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
    TSubclassOf<class UGameplayEffect> CooldownEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    // Fallback montages (only used if AnimBP function isn't available)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> WarriorMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> MageMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> AssassinMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> RangerMontage = nullptr;

    // Hit window settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    FName TraceSocketName = TEXT("WeaponTip");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    float TraceRange = 175.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    float TraceRadius = 25.f;

    // GAS overrides
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    // Gameplay Event callbacks
    UFUNCTION() void OnHitWindowEvent(FGameplayEventData Payload);
    UFUNCTION() void OnAttackEndEvent(FGameplayEventData Payload);
    UFUNCTION() void OnMontageCompleted();

private:
    // Only used by the fallback AbilityTask path
    UPROPERTY() UAbilityTask_PlayMontageAndWait* MontageTask = nullptr;

    bool bConsumedHitWindow = false;

    // Fallback: choose per-class montage if the AnimBP function isn't found
    UAnimMontage* ResolveClassMontage(const FGameplayAbilityActorInfo* ActorInfo) const;
};
