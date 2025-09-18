// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Characters/FS_PlayerClass.h"
#include "Abilities/GameplayAbility.h"
#include "FS_ClassConfig.generated.h"

class UGameplayEffect;

/** One ability grant entry (lets you set Level/InputID per ability if you like) */
USTRUCT(BlueprintType)
struct FFS_AbilityGrant
{
    GENERATED_BODY()

    /** Ability class to grant */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> Ability = nullptr;

    /** Optional: starting level (default 1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Level = 1;

    /** Optional: input binding (use your EFSAbilityInputID values; default = Ability1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 InputID = 0; // set to (int32)EFSAbilityInputID::Ability1 in the DA if desired
};

/** Grouped grants for a class (expandable later if needed) */
USTRUCT(BlueprintType)
struct FFS_ClassGrants
{
    GENERATED_BODY()

    /** Abilities to give this class on spawn */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FFS_AbilityGrant> Abilities;
};

/**
 * Data hub for per-class startup content.
 * - ClassInitEffects: one-shot Instant GE to set starting stats.
 * - ClassPassiveEffects: optional Infinite GE(s) for auras/regen/etc.
 * - ClassDefaultAbilities: abilities to grant on spawn for that class.
 */
UCLASS(BlueprintType)
class FANTASYSURVIVAL_API UFS_ClassConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    /** One-time init effect (Instant) applied when the class is finalized */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Effects")
    TMap<EFSPlayerClass, TSubclassOf<UGameplayEffect>> ClassInitEffects;

    /** Optional passive infinite effect */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Effects")
    TMap<EFSPlayerClass, TSubclassOf<UGameplayEffect>> ClassPassiveEffects;

    /** Abilities granted at spawn for the selected class */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Abilities")
    TMap<EFSPlayerClass, FFS_ClassGrants> ClassDefaultGrants;
};
