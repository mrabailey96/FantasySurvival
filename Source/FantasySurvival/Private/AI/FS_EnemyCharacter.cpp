// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/FS_EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AFS_EnemyCharacter::AFS_EnemyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create ASC and Attributes on the enemy itself
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	StatsSet = CreateDefaultSubobject<UFS_AttributeSet_Stats>(TEXT("StatsSet"));

	// Make sure AI can replicate properly
	bReplicates = true;
	SetNetUpdateFrequency(30.0f);
}

// Called when the game starts or when spawned
void AFS_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize ASC info: Owner = this Actor, Avatar = this pawn
	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this, this);

		// Apply init attributes on the server
		if (HasAuthority() && InitializeAttributesEffect)
		{
			FGameplayEffectContextHandle Context = AbilitySystem->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = AbilitySystem->MakeOutgoingSpec(InitializeAttributesEffect, 1.0f, Context);
			if (Spec.IsValid())
			{
				AbilitySystem->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}

		// Listen for health changes so we can die
		if (StatsSet)
		{
			HealthChangedHandle = AbilitySystem->GetGameplayAttributeValueChangeDelegate(StatsSet->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
				{
					OnHealthChanged(Data);
				});
		}
	}
}

void AFS_EnemyCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (StatsSet && StatsSet->GetHealth() <= 0.0f)
	{
		HandleDeath();
	}
}

void AFS_EnemyCharacter::HandleDeath()
{
	// Simple death: disable collision + hide, then destroy actor
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	// We can play a death anim here; for now, just destroy after a short delay
	SetLifeSpan(2.0f);
}