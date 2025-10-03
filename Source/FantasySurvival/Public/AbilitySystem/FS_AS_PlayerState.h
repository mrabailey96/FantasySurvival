// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "FS_AS_PlayerState.generated.h"

class UFS_AttributeSet_Stats_;

/**
 * APlayerState is the recommended owner of the ASC for characters.
 * - Survives pawn death/respawn
 * - Replicates cleanly for multiplayer
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_AS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AFS_AS_PlayerState();

	/* IAbilitySystemInterface: Returns this player's ASC */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	/* Attributes set accessor */
	UFS_AttributeSet_Stats_* GetStats() const { return StatsSet; }

	/* Convenience getters for UI or debug logging */
	float GetHealth() const;
	float GetMaxHealth() const;
	float GetMana() const;
	float GetMaxMana() const;
	float GetStamina() const;
	float GetMaxStamina() const;

protected:
	/* Core ASC instance that grants/handles abilities/effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	/* Our core attributes (Health/Mana/Stamina) */
	UPROPERTY()
	UFS_AttributeSet_Stats_* StatsSet;
};
