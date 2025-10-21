// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FS_EnemyCombatComponent.generated.h"


class UAbilitySystemComponent;
class UGameplayEffect;
class UAnimMontage;

UENUM(BlueprintType)
enum class EFS_EnemyAttackType : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged") // Not yet implemented in Slice 3.3
};

UCLASS( ClassGroup=(FS), meta=(BlueprintSpawnableComponent) )
class FANTASYSURVIVAL_API UFS_EnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFS_EnemyCombatComponent();

	// Called by AI/Controller to attempt an attack on target. Distance & cooldown gated
	UFUNCTION(BlueprintCallable, Category = "FS|Combat")
	bool TryAttack(AActor* Target);

	// Diretly read by AI/BT if needed
	UFUNCTION(BlueprintPure, Category = "FS|Combat")
	float GetAttackRange() const { return AttackRange; }

	// Optional: Let weapons override at runtime
	UFUNCTION(BlueprintCallable, Category = "FS|Combat")
	void SetAttackRange(float NewRange) { AttackRange = FMath::Clamp(NewRange, 50.0f, 3000.0f); }

	// Callable from AnimNotify at the impact frame
	UFUNCTION(BlueprintCallable, Category = "FS|Combat")
	void PerformMeleeWindow(); // Does the actual sweep + damage when the montage hits

	// Optional: If we plann to use a NotifyState (Multi-frame window)
	UFUNCTION(BlueprintCallable, Category = "FS|Combat")
	void BeginMeleeWindow(); // Clears AlreadyHit, etc.

	UFUNCTION(BlueprintCallable, Category = "FS|Combat")
	void EndMeleeWindow(); // Any cleanup we want

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// GAS & Data
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC = nullptr;

	// GameplayEffect that reduces Health (same one the player uses)
	UPROPERTY(EditDefaultsOnly, Category = "FS|Combat|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// SetByCaller tag used for damage magnitude (e.g., "Data.Damage")
	UPROPERTY(EditDefaultsOnly, Category = "FS|Combat|Damage")
	FGameplayTag SetByCallerDamageTag;

	// Attack Settings
	UPROPERTY(EditAnywhere, Category = "FS|Combat")
	EFS_EnemyAttackType AttackType = EFS_EnemyAttackType::Melee;

	// How close we must be to attack (units). Slice 3.2 seeds this from the enemy
	UPROPERTY(EditAnywhere, Category = "FS|Combat", meta = (ClampMin = "50.0", ClampMax = "3000.0"))
	float AttackRange = 200.0f;

	// Seconds between attacks
	UPROPERTY(EditAnywhere, Category = "FS|Combat", meta = (ClampMin = "0.05"))
	float AttackCooldown = 1.25f;

	// Positive number; We'll negate when applying to Health
	UPROPERTY(EditAnywhere, Category = "FS|Combat|Damage", meta = (ClampMin = "0.0"))
	float DamageBase = 15.0f;

	// Melee Trace
	UPROPERTY(EditAnywhere, Category = "FS|Combat|Melee", meta = (ClampMin = "10.0", ClampMax = "200.0"))
	float MeleeTraceRadius = 40.0f;

	UPROPERTY(EditAnywhere, Category = "FS|Combat|Melee", meta = (ClampMin = "50.0", ClampMax = "400.0"))
	float MeleeTraceForward = 120.0f;

	// FX hooks (GameplayCue Tags we already use)
	UPROPERTY(EditAnywhere, Category = "FS|Combat|FX")
	FGameplayTag DamageNumberCueTag;

	UPROPERTY(EditAnywhere, Category = "FS|Combat|FX")
	FGameplayTag HitReactCueTag;

	// Internals
	double LastAttackTime = -1e9;

	// Depude within a single hit window so a target isnt hit twice by re-sweeps
	TSet<TWeakObjectPtr<AActor>> AlreadyHitThisWindow;

	// Perform the actual melee hit(s) and apply damage via GAS
	bool PerformMelee(AActor* Target);

	// Utility: quick squared distance check
	static bool IsWithinRangeSq(const FVector& From, const FVector& To, float Range);

	void OnAttackMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
