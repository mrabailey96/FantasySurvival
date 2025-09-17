// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_Base.h"

UFS_GA_Base::UFS_GA_Base()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UFS_GA_Base::CommitOrEnd()
{
    FGameplayTagContainer* OptionalRelevantTags = nullptr;
    if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, OptionalRelevantTags))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
        return false;
    }
    return true;
}