// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/FS_PlayerState.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "AbilitySystem/Abilities/FS_GA_Cleave.h"
#include "AbilitySystem/Abilities/FS_GA_ArcaneBolt.h"
#include "AbilitySystem/Abilities/FS_GA_CombatRoll.h"
#include "AbilitySystem/Abilities/FS_GA_ShadowStep.h"
#include "Game/FS_GameInstance.h"

AFS_PlayerState::AFS_PlayerState()
{
	ASC = CreateDefaultSubobject<UFS_AbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	Stats = CreateDefaultSubobject<UFS_AttributeSet_Stats>("Stats");
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AFS_PlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

void AFS_PlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (UFS_GameInstance* GI = GetWorld()->GetGameInstance<UFS_GameInstance>())
		{
			// Use menu-picked class if set
			PlayerClass = GI->PendingClass;
		}

		GrantStartupAbilities();
	}
}

void AFS_PlayerState::GrantStartupAbilities()
{
	check(ASC);

	TSubclassOf<UGameplayAbility> AbilityToGrant = nullptr;

	switch (PlayerClass)
	{
	case EFSPlayerClass::Warrior:  AbilityToGrant = UFS_GA_Cleave::StaticClass(); break;
	case EFSPlayerClass::Mage:     AbilityToGrant = UFS_GA_ArcaneBolt::StaticClass(); break;
	case EFSPlayerClass::Ranger:   AbilityToGrant = UFS_GA_CombatRoll::StaticClass(); break;
	case EFSPlayerClass::Assassin: AbilityToGrant = UFS_GA_ShadowStep::StaticClass(); break;
	}

	if (AbilityToGrant)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(AbilityToGrant, /*Level*/1, /*InputID*/ (int32)EFSAbilityInputID::Ability1, this));
		ASC->InitAbilityActorInfo(this, GetPawn()); // if pawn already exists
	}
}
