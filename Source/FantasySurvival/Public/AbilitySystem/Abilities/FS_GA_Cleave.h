// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FS_GA_Base.h"
#include "FS_GA_Cleave.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GA_Cleave : public UFS_GA_Base
{
	GENERATED_BODY()
	
public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
    UAnimMontage* CleaveMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    float TraceRange = 225.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
    float TraceRadius = 75.f;

    UFUNCTION() void OnCleaveWindow(FGameplayEventData Payload);

    virtual UAnimMontage* ResolveMontage(const FGameplayAbilityActorInfo* ActorInfo) const override
    {
        return CleaveMontage ? CleaveMontage : DefaultMontage.Get();
    }
};
