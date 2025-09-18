// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "FS_GA_PrimaryAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;


UCLASS()
class FANTASYSURVIVAL_API UFS_GA_PrimaryAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UFS_GA_PrimaryAttack();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
    TSubclassOf<class UGameplayEffect> CooldownEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    // Per-class montages (assign in asset or via BP child if you prefer)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> WarriorMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> MageMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> AssassinMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    TObjectPtr<UAnimMontage> RangerMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    FName TraceSocketName = TEXT("WeaponTip");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    float TraceRange = 175.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    float TraceRadius = 25.f;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    UFUNCTION() void OnHitWindowEvent(FGameplayEventData Payload);
    UFUNCTION() void OnMontageCompleted();

private:
    UPROPERTY() UAbilityTask_PlayMontageAndWait* MontageTask = nullptr;
    bool bConsumedHitWindow = false;

    UAnimMontage* ResolveClassMontage(const FGameplayAbilityActorInfo* ActorInfo) const;
};
