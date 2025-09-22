// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_ArcaneBolt.h"
#include "AbilitySystem/FS_NativeTags.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

void UFS_GA_ArcaneBolt::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitOrEnd()) return;

    if (UAbilityTask_WaitGameplayEvent* Wait = WaitForEventTag(TAG_Event_ArcaneBolt_Spawn, false, false))
    {
        Wait->EventReceived.AddDynamic(this, &UFS_GA_ArcaneBolt::OnCastWindow);
        Wait->ReadyForActivation();
    }

    if (!BeginAbilityMontage(ResolveMontage(ActorInfo), 1.f, NAME_None, false))
    {
        OnCastWindow(FGameplayEventData());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UFS_GA_ArcaneBolt::OnCastWindow(FGameplayEventData /*Payload*/)
{
    ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Char || !DamageEffectClass) return;

    const FVector Dir = Char->GetControlRotation().Vector();
    const FVector Start = Char->GetActorLocation() + Dir * 50.f;
    const FVector End = Start + Dir * TraceRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ArcaneBoltTrace), false, Char);
    GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
        FCollisionShape::MakeSphere(TraceRadius), Params);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, 1.25f, 0, 1.25f);
    if (Hit.bBlockingHit) DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 12, FColor::Cyan, false, 1.25f);
#endif

    if (AActor* Target = Hit.GetActor())
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
        {
            FGameplayEffectContextHandle Ctx = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
            Ctx.AddInstigator(Char, Char->GetController());

            FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
            if (Spec.IsValid())
            {
                Spec.Data->SetContext(Ctx);
                TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
    }
}