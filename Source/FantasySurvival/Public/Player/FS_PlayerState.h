// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Characters/FS_PlayerClass.h"
#include "FS_PlayerState.generated.h"

class UFS_AbilitySystemComponent;
class UFS_AttributeSet_Stats;

class UGameplayAbility;
class UGameplayEffect;

/**
 * PlayerState owns ASC + AttributeSet so abilities/attributes persist across pawn swaps.
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFS_PlayerState();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "GAS")
	UFS_AttributeSet_Stats* GetStats() const { return Stats; }

	/** The class chosen in the menu; replicated so UI/HUD can read it on clients */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Class")
	EFSPlayerClass PlayerClass = EFSPlayerClass::Warrior;

	/** Optional: apply this once on BeginPlay to set Max/Current Health/Mana/Stamina, etc. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Init")
	TSubclassOf<UGameplayEffect> InitStatsEffect;

	/** Ability classes per player class (point these to your BP abilities in the PlayerState BP) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> WarriorAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> MageAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> RangerAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> AssassinAbilityClass;

	// AActor
	virtual void BeginPlay() override;

	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** GAS core */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFS_AbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UFS_AttributeSet_Stats> Stats;

	/** Grants the startup ability based on PlayerClass */
	void GrantStartupAbilities();

	/** Debug helper: print class at start */
	void DebugPrintClass() const;
};
