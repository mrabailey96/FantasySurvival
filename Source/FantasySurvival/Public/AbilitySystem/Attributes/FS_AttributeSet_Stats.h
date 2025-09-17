// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "FS_AttributeSet_Stats.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class FANTASYSURVIVAL_API UFS_AttributeSet_Stats : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UFS_AttributeSet_Stats();

	// ---- Health ----
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, Health)

		UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, MaxHealth)

	// ---- Mana ----
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, MaxMana)

	// ---- Stamina ----
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, MaxStamina)

	// Clamp current to max after changes
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Mana(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_MaxMana(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_Stamina(const FGameplayAttributeData& OldValue) const;
	UFUNCTION() void OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const;
};
