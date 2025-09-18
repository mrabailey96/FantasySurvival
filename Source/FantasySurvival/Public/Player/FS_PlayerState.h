// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Characters/FS_PlayerClass.h"
#include "FS_PlayerState.generated.h"

class UAbilitySystemComponent;
class UFS_AbilitySystemComponent;
class UFS_AttributeSet_Stats;
class UFS_ClassConfig;

/**
 * Owns the ASC + AttributeSets for the player and applies the
 * class-specific "Init" GameplayEffect once the ASC is ready.
 *
 * Note:
 *  - Your Character (or Pawn) should call InitializeAbilityActorInfo(...)
 *    after possession so the ASC has correct Owner/Avatar.
 *  - Then call ApplyClassInitialization() once (e.g., in Character::BeginPlay).
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
    AFS_PlayerState();

    /** IAbilitySystemInterface */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UFUNCTION(BlueprintPure, Category = "AbilitySystem")
    UFS_AbilitySystemComponent* GetFSAbilitySystemComponent() const { return AbilitySystemComponent; }

    UFUNCTION(BlueprintPure, Category = "AbilitySystem")
    UFS_AttributeSet_Stats* GetStatsSet() const { return AttributeSetStats; }

    /** Call once after ASC actor info is initialized */
    UFUNCTION(BlueprintCallable, Category = "Class")
    void ApplyClassInitialization();

    /** Helper: Character calls this after possession */
    UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
    void InitializeAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor);

    /** Config mapping Class → effects/abilities (set in defaults/BP) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
    TObjectPtr<UFS_ClassConfig> ClassConfig;

    /** Chosen class (replicates so UI can update) */
    UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_SelectedClass, Category = "Class")
    EFSPlayerClass SelectedClass = EFSPlayerClass::Warrior;

protected:
    /** ASC + AttributeSets live on PlayerState for persistence */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
    TObjectPtr<UFS_AbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
    TObjectPtr<UFS_AttributeSet_Stats> AttributeSetStats;

    /** Guards */
    UPROPERTY(Transient) bool bInitApplied = false;
    UPROPERTY(Transient) bool bAbilitiesGranted = false;

    /** RepNotify */
    UFUNCTION() void OnRep_SelectedClass();

    /** APlayerState */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
