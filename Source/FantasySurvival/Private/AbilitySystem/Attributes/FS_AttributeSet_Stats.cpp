// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Actor.h"

UFS_AttributeSet_Stats::UFS_AttributeSet_Stats()
{
	// Sensible defaults; you can override via GE_Init_Stats:
	InitMaxHealth(100.f);   InitHealth(GetMaxHealth());
	InitMaxMana(100.f);     InitMana(GetMaxMana());
	InitMaxStamina(100.f);  InitStamina(GetMaxStamina());
}

void UFS_AttributeSet_Stats::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	auto ClampToMax = [this](FGameplayAttributeData& Current, const FGameplayAttributeData& Max)
		{
			Current.SetCurrentValue(FMath::Clamp(Current.GetCurrentValue(), 0.f, Max.GetCurrentValue()));
		};

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
		ClampToMax(Health, MaxHealth);
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
		ClampToMax(Mana, MaxMana);
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
		ClampToMax(Stamina, MaxStamina);
}

void UFS_AttributeSet_Stats::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, Health, OldValue);
}
void UFS_AttributeSet_Stats::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, MaxHealth, OldValue);
}
void UFS_AttributeSet_Stats::OnRep_Mana(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, Mana, OldValue);
}
void UFS_AttributeSet_Stats::OnRep_MaxMana(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, MaxMana, OldValue);
}
void UFS_AttributeSet_Stats::OnRep_Stamina(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, Stamina, OldValue);
}
void UFS_AttributeSet_Stats::OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFS_AttributeSet_Stats, MaxStamina, OldValue);
}

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
