// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Characters/FS_PlayerClass.h"
#include "Abilities/GameplayAbility.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstance.h"
#include "FS_ClassConfig.generated.h"

class UGameplayEffect;

/** One ability grant entry (lets us set Level/InputID per ability if we like) */
USTRUCT(BlueprintType)
struct FFS_AbilityGrant
{
    GENERATED_BODY()

    /** Ability class to grant */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> Ability = nullptr;

    /** starting level (default 1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Level = 1;

    /** input binding (use our EFSAbilityInputID values; default = Ability1) */
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

USTRUCT(BlueprintType)
struct FFS_ClassAppearance
{
    GENERATED_BODY()
    /** The skeletal mesh to use for this class */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USkeletalMesh> Mesh;

    /** Anim Instance class (Characters AnimBP's generated class) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UAnimInstance> AnimClass = nullptr;

    /** material overides (index aligned) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<UMaterialInterface>> Materials;


};

USTRUCT(BlueprintType)
struct FFS_WeaponSpec
{
    GENERATED_BODY()

    /** Create a SkeletalMeshComponent */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    /** If set (and SkeletalMesh is null), Create a StaticMeshComponent */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Anim Instance for skeletal weapons (Staff/Wand/Tome with anims, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UAnimInstance> SkeletalAnimClass = nullptr;

    /** Socket to attach to (defaults can be the [ClassName]_weapon_r_socket / [ClassName]_weapon_l_socket) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName AttachSocket = NAME_None;

    /** Per-class per-socket transform overrides (applied relative to socket) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector RelativeScale = FVector(1.f, 1.f, 1.f);

    /** material overrides; index-aligned */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<UMaterialInterface>> Materials;
};

USTRUCT(BlueprintType)
struct FFS_ClassWeapons
{
    GENERATED_BODY()

    /** Right-hand weapon spec */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FFS_WeaponSpec RightHand;

    /** Left-hand weapon spec */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FFS_WeaponSpec LeftHand;
};

/**
 * Data hub for per-class startup content.
 * - ClassInitEffects: one-shot Instant GE to set starting stats.
 * - ClassPassiveEffects: optional Infinite GE(s) for auras/regen/etc.
 * - ClassDefaultAbilities: abilities to grant on spawn for that class.
 * - ClassAppearance/ClassWeapons: Visuals & Attachments
 */
UCLASS(BlueprintType)
class FANTASYSURVIVAL_API UFS_ClassConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    /** One-time init effect (Instant) applied when the class is finalized */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Effects")
    TMap<EFSPlayerClass, TSubclassOf<UGameplayEffect>> ClassInitEffects;

    /** Passive infinite effect */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Effects")
    TMap<EFSPlayerClass, TSubclassOf<UGameplayEffect>> ClassPassiveEffects;

    /** Abilities granted at spawn for the selected class */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Abilities")
    TMap<EFSPlayerClass, FFS_ClassGrants> ClassDefaultGrants;

    /** Mesh/Anim/Materials per class */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Appearance")
    TMap<EFSPlayerClass, FFS_ClassAppearance> ClassAppearance;

    /** Weapon attachment specs per class */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassConfig|Weapons")
    TMap<EFSPlayerClass, FFS_ClassWeapons> ClassWeapons;
};
