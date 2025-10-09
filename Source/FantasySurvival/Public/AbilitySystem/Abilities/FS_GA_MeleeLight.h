// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "FS_GA_MeleeLight.generated.h"

class UGameplayEffect;
class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;

/**
 * Sword Light Attack:
 * - On Activate: play attack montage.
 * - Montage has an Anim Notify "SendGameplayEvent" with tag Event.MeleeHitWindow.
 * - We wait for that gameplay event; when it fires, we do a sphere trace
 *   in front of the character and apply a damage GameplayEffect to hit targets.
 * - We end ability when montage finishes or is canceled.
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GA_MeleeLight : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UFS_GA_MeleeLight();

	// UGameplayAbility Interface
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	// Cost hookes used by CommitAbility()
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// ------- Ability properties -------
	// Attack montage to play for this light swing
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	// GameplayEvent tag the montage will send at the hit window (via Anim Notify)
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Tags")
	FGameplayTag MeleeHitEventTag;

	// Damage Effect (Blueprint GameplayEffect) applied to targets; Uses SetByCaller "Data.Damage"
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Base damage to send as SetByCaller to the damage GE
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Damage")
	float DamageBase = -25.0f;

	// ------- Ability Cost -------
	// GE used to subtract stamina (Instant, SetByCaller Data.StaminaCost)
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Cost")
	TSubclassOf<class UGameplayEffect> StaminaCostEffectClass;

	// How much Stamina this attack costs
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Cost")
	float StaminaCost = 15.0f;

	// ------- Melee Trace -------
	// Trace shape/range; tweak to feel right
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	float TraceRange = 175.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	float TraceRadius = 40.0f;

	// Draw debug for traces
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Debug")
	bool bDebugTrace = true;

	// ------- Hit-react control -------
	// Cooldown GE that grants Cooldown.HitReact for a short duration
	UPROPERTY(EditDefaultsOnly, Category = "Melee|HitReact")
	TSubclassOf<class UGameplayEffect> HitReactCooldownEffectClass;

	// GameplayCue we fire to play the enemy hit react montage
	UPROPERTY(EditDefaultsOnly, Category = "Melee|HitReact")
	FGameplayTag HitReactCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.HitReact.Melee"));

	// ------- Damage Number Cue -------
	UPROPERTY(EditDefaultsOnly, Category = "Melee|UI")
	FGameplayTag DamageNumberCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.UI.DamageNumber"));

private:
	// Keep track of already-hit actors for this swing so we dont double apply
	TSet<TWeakObjectPtr<AActor>> HitActorsThisActivation;

	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* PlayTaskRef = nullptr;

	// Called when the hit window gameplay event fires
	UFUNCTION()
	void OnMeleeHitEvent(FGameplayEventData Payload);

	// Actually performs the sphere trace and applies damage to valid actors
	void DoMeleeHit(const FGameplayAbilityActorInfo* ActorInfo);

	// Ends the ability when the montage finishes
	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnMontageCancelled() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); }

	UFUNCTION()
	void OnMontageCompleted() { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); }
};