// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_CombatRoll.h"
#include "GameFramework/Character.h"

void UFS_GA_CombatRoll::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitOrEnd()) return;

    if (ACharacter* Char = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
    {
        const FVector Dir = Char->GetActorForwardVector();
        Char->LaunchCharacter(Dir * ForwardImpulse + FVector(0, 0, UpImpulse), /*XY*/true, /*Z*/true);
    }

    if (!BeginAbilityMontage(ResolveMontage(ActorInfo), 1.f, NAME_None, false))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}