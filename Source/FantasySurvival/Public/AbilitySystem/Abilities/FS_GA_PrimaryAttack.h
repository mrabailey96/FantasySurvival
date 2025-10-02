// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FS_GA_Base.h"
#include "GameplayTagContainer.h"
#include "FS_GA_PrimaryAttack.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;


UCLASS()
class FANTASYSURVIVAL_API UFS_GA_PrimaryAttack : public UFS_GA_Base
{
	GENERATED_BODY()
	
public:
	UFS_GA_PrimaryAttack();

	/** Attack montage to play */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** GameplayEffect (Instant) that modifies Health using SetByCaller "Data.Damage" */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	/** Positive number; we’ll pass -DamageAmount into SetByCaller so Health goes down if GE is Additive */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Damage", meta = (ClampMin = "0.0"))
	float DamageAmount = 20.f;

	/** Sweep distance forward from socket/origin */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace", meta = (ClampMin = "0.0"))
	float HitRange = 120.f;

	/** Sweep radius */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace", meta = (ClampMin = "1.0"))
	float HitRadius = 80.f;

	/** Which collision channel to sweep against (commonly ECC_Pawn) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	/** Primary socket (usually weapon/hand) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	FName RightHandSocket = FName(TEXT("hand_r_socket"));

	/** Optional offhand; used if bSweepLeftHandToo is true and socket exists */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	FName LeftHandSocket = FName(TEXT("hand_l_socket"));

	/** If true, performs a second sweep from LeftHandSocket in the same frame */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Trace")
	bool bSweepLeftHandToo = false;

	/** Gameplay Event tag emitted by the Anim Notify at the hit frame (must match notify) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Events")
	FGameplayTag MeleeHitEventTag;

protected:
	// UGameplayAbility
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/** Called by FS_GA_Base’s montage delegate bridge when the montage cleanly finishes */
	virtual void OnMontageCompleted() override;

private:
	/** Bound to WaitGameplayEvent::EventReceived */
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	/** Runs the trace(s) and applies the Damage GE to unique targets */
	void DoHitSweepAndApply(const FGameplayAbilityActorInfo* ActorInfo);

	/** Local cache so we don’t hit the same target twice in a single attack window */
	TSet<TWeakObjectPtr<AActor>> AlreadyHit;

};
