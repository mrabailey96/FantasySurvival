// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FS_GA_Base.h"
#include "FS_GA_ArcaneBolt.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GA_ArcaneBolt : public UFS_GA_Base
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// Cast montage (set in BP)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
	UAnimMontage* CastMontage = nullptr;

	// What damage to apply when the bolt hits
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Trace setup (cheap "projectile" stand-in)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
	float TraceRange = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit")
	float TraceRadius = 16.0f;

	// Event handler fired by the montage notify
	UFUNCTION() void OnCastWindow(FGameplayEventData Payload);

	// Per ability montage resolver (uses CastMontage)
	virtual UAnimMontage* ResolveMontage(const FGameplayAbilityActorInfo* ActorInfo) const override
	{
		return CastMontage ? CastMontage : DefaultMontage.Get();
	}
};
