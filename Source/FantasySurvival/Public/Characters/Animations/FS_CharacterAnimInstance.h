// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Characters/FS_PlayerClass.h"
#include "FS_CharacterAnimInstance.generated.h"

class AFS_Character;
class AFS_PlayerState;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FPrimaryAttackAnimSet
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) UAnimMontage* WarriorMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) UAnimMontage* MageMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) UAnimMontage* RangerMontage = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) UAnimMontage* AssassinMontage = nullptr;
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class FANTASYSURVIVAL_API UFS_CharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
    /** Data you fill in the ABP defaults */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    FPrimaryAttackAnimSet PrimaryAttackAnimSet;

    /** Chosen at runtime from the set above */
    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    UAnimMontage* PrimaryAttackMontage = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Attack")
    EFSPlayerClass ClassType = EFSPlayerClass::Warrior;

    /** Play-rate + trigger flag (the GA sets these) */
    UPROPERTY(BlueprintReadWrite, Category = "Attack")
    float AttackPlayRate = 1.f;

    UPROPERTY(BlueprintReadWrite, Category = "Attack")
    bool bWantsPrimaryAttack = false;

    /** Called by GA_PrimaryAttack */
    UFUNCTION(BlueprintCallable, Category = "Attack")
    void StartPrimaryAttack(float InPlayRate = 1.f);

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    void RefreshClassFromOwner();           // reads PlayerState.SelectedClass
    void SelectPrimaryAttackMontage();      // picks montage by ClassType

private:
    TWeakObjectPtr<AFS_Character> CachedCharacter;
    bool bTriedInitClass = false;
	
};
