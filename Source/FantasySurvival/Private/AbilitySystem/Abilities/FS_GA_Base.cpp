// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_Base.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

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

bool UFS_GA_Base::BeginAbilityMontage(UAnimMontage* Montage, float Rate, FName StartSection, bool bStopWhenAbilityEnds)
{
    if (!Montage)
        return false;

    ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, Rate, StartSection, bStopWhenAbilityEnds);

    if (!ActiveMontageTask)
        return false;

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UFS_GA_Base::_OnMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UFS_GA_Base::_OnMontageInterrupted);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UFS_GA_Base::_OnMontageCancelled);
    ActiveMontageTask->ReadyForActivation();
    return true;
}

UAbilityTask_WaitGameplayEvent* UFS_GA_Base::WaitForEventTag(FGameplayTag EventTag, bool bOnlyTriggerOnce, bool bOnlyMatchExact)
{
    return UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, /*OptionalExternalOwner*/nullptr, bOnlyTriggerOnce, bOnlyMatchExact);
}

void UFS_GA_Base::_OnMontageCompleted()
{
    OnMontageCompleted(); // Child Hook
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/false, /*bWasCancelled=*/false);
}

void UFS_GA_Base::_OnMontageInterrupted()
{
    OnMontageCompleted(); // Child Hook
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/false, /*bWasCancelled=*/true);
}

void UFS_GA_Base::_OnMontageCancelled()
{
    OnMontageCompleted(); // Child Hook
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/false, /*bWasCancelled=*/true);
}
