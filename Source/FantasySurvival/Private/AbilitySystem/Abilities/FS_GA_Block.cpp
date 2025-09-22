// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_Block.h"
#include "AbilitySystemComponent.h"

static FGameplayTag TAG_State_Blocking = FGameplayTag::RequestGameplayTag(TEXT("State.Blocking"));
static FGameplayTag TAG_State_Attacking = FGameplayTag::RequestGameplayTag(TEXT("State.Attacking"));

UFS_GA_Block::UFS_GA_Block()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true; // Ensures release arrives on server too
	ActivationOwnedTags.AddTag(TAG_State_Blocking); // Player owns "Blocking" while active
	ActivationBlockedTags.AddTag(TAG_State_Attacking); // Cant start if currently attacking
}

void UFS_GA_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitOrEnd()) return;

	// Optional if applying a Damage Reduction Gameplay Effect while blocking
	if (BlockingEffectClass)
	{
		FGameplayEffectSpecHandle DR = MakeOutgoingGameplayEffectSpec(BlockingEffectClass, GetAbilityLevel());
		if (DR.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DR);
		}
	}

	// it is recommended to drive a blocking pose via AnimBP using the State.Blocking tag.
	// If we still want a quick "raise shield" montage, play it our own risk with our own task here
	// and do NOT end the ability on completion
}

void UFS_GA_Block::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/true, /*bWasCancelled*/false);
}

void UFS_GA_Block::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clean up DR effect if we applied one
	if (BlockingEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(BlockingEffectHandle);
		}
		BlockingEffectHandle = FActiveGameplayEffectHandle();
	}

	// Remove temporary effects if applied (DR buff)
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
