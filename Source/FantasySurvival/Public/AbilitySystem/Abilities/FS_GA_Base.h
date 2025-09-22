// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/FS_AbilityInputID.h"
#include "FS_GA_Base.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GA_Base : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFS_GA_Base();

protected:
	// ---- Common Properties ----
	// Let specs carry which input they use
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	EFSAbilityInputID InputID = EFSAbilityInputID::Ability1;

	// Optional default montage if a child ability doesnt override ResolveMontage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Animation")
	TObjectPtr<UAnimMontage> DefaultMontage = nullptr;

	// The active montage task (for convenience/inspection)
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* ActiveMontageTask = nullptr;

	// ---- Helpers ----
	// Utility to commit (checks cost/cooldown) safely
	bool CommitOrEnd();

	// Resolve which montage to play (children can override)
	virtual UAnimMontage* ResolveMontage(const FGameplayAbilityActorInfo* ActorInfo) const { return DefaultMontage; }

	// Begin playing a montage with standard bindings. Returns false on failure
	bool BeginAbilityMontage(UAnimMontage* Montage, float Rate = 1.0f, FName StartSection = NAME_None, bool bStopWhenAbilityEnds = false);

	// Simple wrapper to create a WaitGameplayEvent task (Child binds its own handler)
	UAbilityTask_WaitGameplayEvent* WaitForEventTag(FGameplayTag EventTag, bool bOnlyTriggerOncec = false, bool bOnlyMatchExact = false);

	// ---- Montage delegate bridge (UFUNCTION so we can bind dynamic delegates) ----
	UFUNCTION() void _OnMontageCompleted();
	UFUNCTION() void _OnMontageInterrupted();
	UFUNCTION() void _OnMontageCancelled();

	// Virtual hooks for children that need extra work on these moments
	virtual void OnMontageCompleted() {}
	virtual void OnMontageInterrupted() {}
	virtual void OnMontageCancelled() {}
};
