// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/FS_EnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "UI/FS_EnemyHealthBarWidget.h"
#include "UObject/UnrealType.h"
#include "Combat/FS_EnemyCombatComponent.h"

// Sets default values
AFS_EnemyCharacter::AFS_EnemyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Make sure AI can replicate properly
	bReplicates = true;
	SetNetUpdateFrequency(30.0f);

	// Movement basics (Tweak if needed)
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;

	}

	// Create ASC and Attributes on the enemy itself
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	StatsSet = CreateDefaultSubobject<UFS_AttributeSet_Stats>(TEXT("Stats_Enemy"));

	CombatComp = CreateDefaultSubobject<UFS_EnemyCombatComponent>(TEXT("CombatComponent"));
	// Seed component's range from our per-enemy property
	if (CombatComp) CombatComp->SetAttackRange(AttackRange);

	// Health bar widget (We'll set the widget class in the BP)
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World);
	HealthBarWidget->SetDrawSize(FVector2D(120.0f, 12.0f));
	HealthBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // Height above head
	HealthBarWidget->SetTwoSided(true);

	// AI Possesion for placed/spawned pawns
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

// Called when the game starts or when spawned
void AFS_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystem) return;

	// Required so ASC knows its owner/avatar
	AbilitySystem->InitAbilityActorInfo(this, this);

	// Apply initial attributes on the server
	if (HasAuthority() && InitializeAttributesEffect)
	{
		const FGameplayEffectContextHandle Context = AbilitySystem->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = AbilitySystem->MakeOutgoingSpec(InitializeAttributesEffect, 1.0f, Context);
		if (Spec.IsValid())
		{
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	// Bind Health Change Delegate (Death Gate)
	{
		const FGameplayAttribute HealthAttr = UFS_AttributeSet_Stats::GetHealthAttribute();
		HealthChangedHandle = AbilitySystem->GetGameplayAttributeValueChangeDelegate(HealthAttr).AddUObject(this, &AFS_EnemyCharacter::OnHealthChanged);
	}

	// Hook widget logic (hands ASC to the UFS_EnemyHealthBarWidget and it binds itself)
	InitHealthBarWidget();

	// Spawn-dead edge case (e.g., bad init data)
	if (StatsSet && StatsSet->GetHealth() <= 0.0f)
	{
		HandleDeath();
	}
}

void AFS_EnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind Health delegate cleanly
	if (AbilitySystem && HealthChangedHandle.IsValid())
	{
		const FGameplayAttribute HealthAttr = UFS_AttributeSet_Stats::GetHealthAttribute();
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(HealthAttr).Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void AFS_EnemyCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// Use payload value; Avoids extra ASC reads during teardown
	if (!bIsDead && Data.NewValue <= 0.0f)
	{
		HandleDeath();
	}
}

void AFS_EnemyCharacter::HandleDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	// Disable movement & collision
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	SetActorEnableCollision(false);

	// BP hook for SFX/VFX
	BP_OnDeath();

	// Cleanup
	SetLifeSpan(CorpseLifespan);
}

void AFS_EnemyCharacter::InitHealthBarWidget()
{
	if (!HealthBarWidget) return;

	if (UUserWidget* UW = HealthBarWidget->GetUserWidgetObject())
	{
		// Direct cast to our C++ widget subclass
		if (UFS_EnemyHealthBarWidget* HB = Cast<UFS_EnemyHealthBarWidget>(UW))
		{
			HB->InitializeFromASC(AbilitySystem);

			//// If health == MaxHealth on Spawn, also hide the WidgetComponent for good measure
			//const float H = AbilitySystem->GetNumericAttribute(UFS_AttributeSet_Stats::GetHealthAttribute());
			//const float Max = AbilitySystem->GetNumericAttribute(UFS_AttributeSet_Stats::GetMaxHealthAttribute());
			//if (Max > 0.0f && FMath::IsNearlyEqual(H / Max, 1.0f, 0.001f))
			//{
			//	HealthBarWidget->SetVisibility(false); // Component level hide
			//}
		}
	}
}