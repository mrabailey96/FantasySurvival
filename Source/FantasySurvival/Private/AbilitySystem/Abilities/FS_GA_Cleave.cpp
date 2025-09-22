// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_Cleave.h"
#include "AbilitySystem/FS_NativeTags.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

void UFS_GA_Cleave::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitOrEnd()) return;

    if (UAbilityTask_WaitGameplayEvent* Wait = WaitForEventTag(TAG_Event_Cleave_HitWindow, false, false))
    {
        Wait->EventReceived.AddDynamic(this, &UFS_GA_Cleave::OnCleaveWindow);
        Wait->ReadyForActivation();
    }

    if (!BeginAbilityMontage(ResolveMontage(ActorInfo), 1.f, NAME_None, false))
    {
        OnCleaveWindow(FGameplayEventData());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UFS_GA_Cleave::OnCleaveWindow(FGameplayEventData /*Payload*/)
{
    ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Char || !DamageEffectClass) return;

    const FVector Start = Char->GetActorLocation() + Char->GetActorForwardVector() * 50.f;
    const FVector End = Start + Char->GetActorForwardVector() * TraceRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CleaveTrace), false, Char);
    GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(TraceRadius), Params);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1.25f, 0, 1.5f);
    if (Hit.bBlockingHit) DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 16, FColor::Yellow, false, 1.25f);
#endif

    if (AActor* Target = Hit.GetActor())
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
        {
            FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
            if (Spec.IsValid())
            {
                TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
    }
}
