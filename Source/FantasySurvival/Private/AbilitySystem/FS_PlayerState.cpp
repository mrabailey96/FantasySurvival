// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/FS_PlayerState.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"

AFS_PlayerState::AFS_PlayerState()
{
	// Create the ASC as a default subobject so it exists on all instances (server & client)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// Recommended replication settings for PlayerState-owned ASC:
	// Mixed mode gives good results for predicting the owner while replicating to others
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create our AttributeSet; It's a UObject that the ASC tracks
	StatsSet = CreateDefaultSubobject<UFS_AttributeSet_Stats>(TEXT("StateSet"));

	// Options: PlayerState Defaults
	SetNetUpdateFrequency(60.0f);
}

float AFS_PlayerState::GetHealth() const
{
	return StatsSet ? StatsSet->GetHealth() : 0.0f;
}

float AFS_PlayerState::GetMaxHealth() const
{
	return StatsSet ? StatsSet->GetMaxHealth() : 0.0f;
}

float AFS_PlayerState::GetMana() const
{
	return StatsSet ? StatsSet->GetMana() : 0.0f;
}

float AFS_PlayerState::GetMaxMana() const
{
	return StatsSet ? StatsSet->GetMaxMana() : 0.0f;
}

float AFS_PlayerState::GetStamina() const
{
	return StatsSet ? StatsSet->GetStamina() : 0.0f;
}

float AFS_PlayerState::GetMaxStamina() const
{
	return StatsSet ? StatsSet->GetMaxStamina() : 0.0f;
}
