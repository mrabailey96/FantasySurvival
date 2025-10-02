// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FS_DamageDummy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"

// Sets default values
AFS_DamageDummy::AFS_DamageDummy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create ASC & AttributeSet
	AbilitySystem = CreateDefaultSubobject<UFS_AbilitySystemComponent>(TEXT("AbilitySystem"));
	Stats = CreateDefaultSubobject<UFS_AttributeSet_Stats>(TEXT("Stats"));

	// Basic collision so melee sphere sweeps (ECC_Pawn) can hit us
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	// Prevent Moving
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;

}

UAbilitySystemComponent* AFS_DamageDummy::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

// Called when the game starts or when spawned
void AFS_DamageDummy::BeginPlay()
{
	Super::BeginPlay();

	// Make sure ASC knows its actor info (Owner = this, Avatar = this)
	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this, this);
	}

	ApplyInitialAttributes();

	// Bind to Health changed (requires out AttributeSet exposes the Health attribute)
	if (AbilitySystem && Stats)
	{
		const FGameplayAttribute HealthAttr = Stats->GetHealthAttribute(); // Access - Adjust if API differs
		AbilitySystem->GetGameplayAttributeValueChangeDelegate(HealthAttr).AddLambda([this](const FOnAttributeChangeData& Data)
			{
				HandleHealthChanged(Data.NewValue, Data.OldValue);
			});
	}
	
}

void AFS_DamageDummy::ApplyInitialAttributes()
{
	if (!AbilitySystem || !DefaultAttributes) return;

	FGameplayEffectContextHandle Ctx = AbilitySystem->MakeEffectContext();
	Ctx.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = AbilitySystem->MakeOutgoingSpec(DefaultAttributes, 1, Ctx);
	if (Spec.IsValid())
	{
		AbilitySystem->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void AFS_DamageDummy::HandleHealthChanged(float NewValue, float OldValue)
{
	UE_LOG(LogTemp, Log, TEXT("Dummy Health: %.1f -> %.1f"), OldValue, NewValue);

	if (NewValue <= 0.0f)
	{
		// Death
		if (bRagdollOnDeath && GetMesh())
		{
			GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetSimulatePhysics(true);
			GetMesh()->WakeAllRigidBodies();

			GetCharacterMovement()->DisableMovement();
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			// Clean up later
			GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AFS_DamageDummy::DelayedDestroy, RagdollLifeSeconds, false);
		}
		else
		{
			Destroy();
		}
	}
}

void AFS_DamageDummy::DelayedDestroy()
{
	Destroy();
}
