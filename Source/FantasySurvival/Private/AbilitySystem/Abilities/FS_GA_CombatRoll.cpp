// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_CombatRoll.h"
#include "GameFramework/Character.h"

void UFS_GA_CombatRoll::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitOrEnd()) return;
	if (ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		const FVector Dir = Char->GetActorForwardVector();
		Char->LaunchCharacter(Dir * 600.f + FVector(0, 0, 100.f), true, true);
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
