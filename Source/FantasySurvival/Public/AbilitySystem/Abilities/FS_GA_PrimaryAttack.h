// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/FS_GA_Base.h"
#include "FS_GA_PrimaryAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;


UCLASS()
class FANTASYSURVIVAL_API UFS_GA_PrimaryAttack : public UFS_GA_Base
{
	GENERATED_BODY()
	
public:
	UFS_GA_PrimaryAttack();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TSubclassOf<class UGameplayEffect> CooldownEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	// Per-Class Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation") UAnimMontage* WarriorMontage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation") UAnimMontage* MageMontage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation") UAnimMontage* AssassinMontage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation") UAnimMontage* RangerMontage = nullptr;

	// Hit window settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit") FName TraceSocketName = TEXT("WeaponTip");
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit") float TraceRange = 175.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Hit") float TraceRadius = 25.0f;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// Event handlers
	UFUNCTION() void OnHitWindowEvent(FGameplayEventData Payload);

	// Choose the right montage for the current class
	virtual UAnimMontage* ResolveMontage(const FGameplayAbilityActorInfo* ActorInfo) const override;

private:
	UPROPERTY() bool bConsumedHitWindow = false;

};
