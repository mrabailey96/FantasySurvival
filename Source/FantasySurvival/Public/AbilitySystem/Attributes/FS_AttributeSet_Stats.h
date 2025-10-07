// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "FS_AttributeSet_Stats.generated.h"

// Macro to create attribute accessors (Getters, Setters, Initializers)
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UFS_AttributeSet_Stats
 *
 * This AttributeSet holds the CORE PLAYER STATS:
 * - Health (survivability)
 * - Mana (resource for casting, ranged power shots, magic weapons)
 * - Stamina (resource for melee, dodging, sprinting)
 *
 * Each attribute will also have a Max value (so effects can scale them).
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_AttributeSet_Stats : public UAttributeSet
{
    GENERATED_BODY()

public:
    UFS_AttributeSet_Stats();

    // Called when an attribute is changed (e.g., Health reduced by damage)
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

    /** ----------------- Health ----------------- */
    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, Health);

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, MaxHealth);


    /** ----------------- Mana ----------------- */
    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Mana)
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, Mana);

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxMana)
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, MaxMana);


    /** ----------------- Stamina ----------------- */
    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Stamina)
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, Stamina);

    UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_MaxStamina)
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UFS_AttributeSet_Stats, MaxStamina);

protected:
    /** Replication functions for Multiplayer */
    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

    UFUNCTION()
    void OnRep_Mana(const FGameplayAttributeData& OldMana);

    UFUNCTION()
    void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

    UFUNCTION()
    void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

    UFUNCTION()
    void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

};