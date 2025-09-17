// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Characters/FS_PlayerClass.h"
#include "FS_PlayerState.generated.h"

class UFS_AbilitySystemComponent;
class UFS_AttributeSet_Stats;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFS_PlayerState();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable) UFS_AttributeSet_Stats* GetStats() const { return Stats; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	EFSPlayerClass PlayerClass = EFSPlayerClass::Warrior;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFS_AbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UFS_AttributeSet_Stats> Stats;

	virtual void BeginPlay() override;

	// Grants default per-class ability
	void GrantStartupAbilities();
};
