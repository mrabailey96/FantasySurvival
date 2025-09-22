// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FS_GA_Base.h"
#include "FS_GA_CombatRoll.generated.h"

class UAnimMontage;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GA_CombatRoll : public UFS_GA_Base
{
	GENERATED_BODY()
	
public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    UAnimMontage* RollMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Roll")
    float ForwardImpulse = 600.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Roll")
    float UpImpulse = 100.f;

    virtual UAnimMontage* ResolveMontage(const FGameplayAbilityActorInfo* ActorInfo) const override
    {
        return RollMontage ? RollMontage : DefaultMontage.Get();
    }
};
