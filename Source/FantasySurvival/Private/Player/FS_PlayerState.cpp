// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/FS_PlayerState.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"

#include "AbilitySystem/Abilities/FS_GA_Cleave.h"
#include "AbilitySystem/Abilities/FS_GA_ArcaneBolt.h"
#include "AbilitySystem/Abilities/FS_GA_CombatRoll.h"
#include "AbilitySystem/Abilities/FS_GA_ShadowStep.h"

#include "Game/FS_GameInstance.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

AFS_PlayerState::AFS_PlayerState()
{
	// Create and configure ASC + AttributeSet on the PlayerState
	ASC = CreateDefaultSubobject<UFS_AbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	Stats = CreateDefaultSubobject<UFS_AttributeSet_Stats>(TEXT("Stats"));

	// Use function (not direct property) per 5.6 deprecation note
	SetNetUpdateFrequency(100.f);

	// Sensible defaults for ability classes; override in BP if you want the BP versions
	WarriorAbilityClass = UFS_GA_Cleave::StaticClass();
	MageAbilityClass = UFS_GA_ArcaneBolt::StaticClass();
	RangerAbilityClass = UFS_GA_CombatRoll::StaticClass();
	AssassinAbilityClass = UFS_GA_ShadowStep::StaticClass();
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
		// Pull the class chosen in the menu (GameInstance) if available
		if (UFS_GameInstance* GI = GetWorld()->GetGameInstance<UFS_GameInstance>())
		{
			PlayerClass = GI->PendingClass;
		}

		// Apply initial stats once, if provided (lets you tune in BP)
		if (InitStatsEffect && ASC)
		{
			ASC->ApplyGameplayEffectToSelf(InitStatsEffect.GetDefaultObject(), /*Level*/1.0f, ASC->MakeEffectContext());
		}

		GrantStartupAbilities();
	}

	DebugPrintClass();
}

void AFS_PlayerState::GrantStartupAbilities()
{
	if (!ASC || !HasAuthority())
	{
		return;
	}

	TSubclassOf<UGameplayAbility> AbilityToGrant = nullptr;

	switch (PlayerClass)
	{
	case EFSPlayerClass::Warrior:   AbilityToGrant = WarriorAbilityClass;  break;
	case EFSPlayerClass::Mage:      AbilityToGrant = MageAbilityClass;     break;
	case EFSPlayerClass::Ranger:    AbilityToGrant = RangerAbilityClass;   break;
	case EFSPlayerClass::Assassin:  AbilityToGrant = AssassinAbilityClass; break;
	default: break;
	}

	if (AbilityToGrant)
	{
		// InputID = Ability1 (so your Enhanced Input press triggers it)
		const int32 InputID = static_cast<int32>(EFSAbilityInputID::Ability1);
		ASC->GiveAbility(FGameplayAbilitySpec(AbilityToGrant, /*Level*/1, /*InputID*/InputID, this));
		// ActorInfo is initialized by your Character (AFS_Character) in PossessedBy/OnRep_PlayerState.
	}
}

void AFS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFS_PlayerState, PlayerClass);
}

void AFS_PlayerState::DebugPrintClass() const
{
#if !UE_BUILD_SHIPPING
	static auto ToString = [](EFSPlayerClass C)
		{
			return StaticEnum<EFSPlayerClass>()->GetNameStringByValue(static_cast<int64>(C));
		};

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.f, FColor::Green,
			FString::Printf(TEXT("FS_PlayerState :: PlayerClass = %s"), *ToString(PlayerClass)));
	}
#endif
}