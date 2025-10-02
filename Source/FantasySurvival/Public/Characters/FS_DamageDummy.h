// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "FS_DamageDummy.generated.h"

class UFS_AbilitySystemComponent;
class UFS_AttributeSet_Stats;
class UGameplayEffect;

UCLASS()
class FANTASYSURVIVAL_API AFS_DamageDummy : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFS_DamageDummy();

	// IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The projects ASC (replicated by default if our class does so)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFS_AbilitySystemComponent> AbilitySystem;

	// The core stats AttributeSet (Health, Mana, Stamina)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UFS_AttributeSet_Stats> Stats;

	// Initial attributes to apply on BeginPlay (Instant or Duration = Infinite)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributes;

	// If true, the dummy will ragdoll on death; otherwise Destroy()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dummy")
	bool bRagdollOnDeath = true;

	// Minimum time before Destroy() after ragdolling (seconds)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dummy", meta = (EditCondition = "bRagdollOnDeath"))
	float RagdollLifeSeconds = 6.0f;

private:
	void ApplyInitialAttributes();

	//Attribute changle handlers
	void HandleHealthChanged(float NewValue, float OldValue);

	FTimerHandle DestroyTimerHandle;
	void DelayedDestroy();

};
