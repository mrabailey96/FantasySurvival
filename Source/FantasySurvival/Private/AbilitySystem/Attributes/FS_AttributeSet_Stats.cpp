// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UFS_AttributeSet_Stats::UFS_AttributeSet_Stats()
{
	/* Default values - these can be overridden by a GameplayEffect (GE_InitializeAttributes) */

	Health = 100.0f;
	MaxHealth = 100.0f;

	Mana = 50.0f;
	MaxMana = 50.0f;

	Stamina = 75.0f;
	MaxStamina = 75.0f;
}

void UFS_AttributeSet_Stats::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// This function runs whenever an attribute is changed
	// Example: Clamp Health between 0 and MaxHealth
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}

/** ------------------ Replication Setup ------------------ */
void UFS_AttributeSet_Stats::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, Health, OldHealth);
}

void UFS_AttributeSet_Stats::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, MaxHealth, OldMaxHealth);
}

void UFS_AttributeSet_Stats::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, Mana, OldMana);
}

void UFS_AttributeSet_Stats::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, MaxMana, OldMaxMana);
}

void UFS_AttributeSet_Stats::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, Stamina, OldStamina);
}

void UFS_AttributeSet_Stats::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, MaxStamina, OldMaxStamina);
}

/* Required for Replication */
void UFS_AttributeSet_Stats::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UFS_AttributeSet_Stats, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFS_AttributeSet_Stats, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UFS_AttributeSet_Stats, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFS_AttributeSet_Stats, MaxMana, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UFS_AttributeSet_Stats, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFS_AttributeSet_Stats, MaxStamina, COND_None, REPNOTIFY_Always);
}
