// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/FS_AbilityInputID.h"
#include "FS_GA_Base.generated.h"

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GA_Base : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFS_GA_Base();

protected:
	// Let specs carry which input they use
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	EFSAbilityInputID InputID = EFSAbilityInputID::Ability1;

	// Utility to commit (checks cost/cooldown) safely
	bool CommitOrEnd();
};
