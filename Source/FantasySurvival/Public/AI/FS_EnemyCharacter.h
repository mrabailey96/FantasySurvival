// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include <GameplayEffectTypes.h>
#include "Components/WidgetComponent.h"
#include "FS_EnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UFS_AttributeSet_Stats;
class UGameplayEffect;

/**
 * Very simple enemy that:
 * - Owns its own ASC (AI typically owns the ASC on itself, not PlayerState).
 * - Has the same Stats AttributeSet as the player.
 * - Applies an init GE on BeginPlay (server).
 * - Destroys itself on Health <= 0.
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_EnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFS_EnemyCharacter();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }

	// BP hook for SFX/VFX on death
	UFUNCTION(BlueprintImplementableEvent, Category = "FS|Enemy")
	void BP_OnDeath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Enemy's ASC + Attributes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFS_AttributeSet_Stats> StatsSet;

	// GE to Init Health/Mana/Stamina (Use the same GE_InitializeAttributes as player)
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffect;

	// Delagate handle for health change so we detect death
	FDelegateHandle HealthChangedHandle;

	// World-Space health bar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarWidget;

	// Soft death guard + lifespan
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, Category = "State")
	float CorpseLifespan = 8.0f;

	// Callback for health changes
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Kill/Destroy self when health reaches zero
	void HandleDeath();

	// Push ASC into the widget and call its Init() if present
	void InitHealthBarWidget();

};
