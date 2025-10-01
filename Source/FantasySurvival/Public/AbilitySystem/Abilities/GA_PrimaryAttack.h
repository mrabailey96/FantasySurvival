// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PrimaryAttack.generated.h"

class UAnimMontage;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UGA_PrimaryAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
    UGA_PrimaryAttack();

    // UGameplayAbility
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    /** Play this when we attack */
    UPROPERTY(EditDefaultsOnly, Category = "Attack")
    TObjectPtr<UAnimMontage> AttackMontage;

    /** Instant damage GE (uses SetByCaller "Data.Damage") */
    UPROPERTY(EditDefaultsOnly, Category = "Attack")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    /** Positive number; we’ll send -Damage to the GE */
    UPROPERTY(EditDefaultsOnly, Category = "Attack")
    float Damage = 25.f;

    /** Sweep settings */
    UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
    float HitRange = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
    float HitRadius = 28.f;

    /** Socket names to try (right/left hand) */
    UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
    FName RightHandSocket = FName("weapon_r_socket");

    UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
    FName LeftHandSocket = FName("weapon_l_socket");

    /** Which channel to sweep on (Pawn is typical) */
    UPROPERTY(EditDefaultsOnly, Category = "Attack|Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

private:
    /** Track who we've hit this activation so we don't double-apply */
    TSet<TWeakObjectPtr<AActor>> AlreadyHit;

    /** Callbacks */
    UFUNCTION() void OnMontageCompleted();
    UFUNCTION() void OnMontageInterrupted();
    UFUNCTION() void OnMontageCancelled();
    UFUNCTION() void OnHitEventReceived(FGameplayEventData Payload);

    void DoHitSweepAndApply(const FGameplayAbilityActorInfo* ActorInfo);
	
};
