// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FS_EnemyAnimInstance.generated.h"

class ACharacter;
class UBlendSpace;
class UAnimMontage;

/**
 * Native AnimInstance that computes locomotion vars.
 * The AnimGraph (in a small Anim Blueprint asset) reads these vars.
 */
UCLASS(Blueprintable, BlueprintType)
class FANTASYSURVIVAL_API UFS_EnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UFS_EnemyAnimInstance();

	// Current planar speed (cm/s)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FS|Locomotion")
	float Speed = 0.0f;

	// Direction in degress relative to actor rotation: [-180..180],  where 0 = forward
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FS|Locomotion")
	float Direction = 0.0f;

	// Is the character in the air (falling)?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FS|Locomotion")
	bool bIsInAir = false;

	// Expose for convenience in BP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FS|Locomotion")
	FRotator ActorRotation = FRotator::ZeroRotator;

	// Locomotion BlendSpace(2D). The AnimBP graph will us this
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FS|Assets")
	TObjectPtr<UBlendSpace> LocomotionBS = nullptr;

	// Upper-body slot montage for melee (used by out CombatComponent)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FS|Assets")
	TObjectPtr<UAnimMontage> MeleeMontage = nullptr;

protected:
	// UAnimInstance
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	// Cached owning character for quick access
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter = nullptr;
};
