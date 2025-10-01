// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/GA_PrimaryAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

UGA_PrimaryAttack::UGA_PrimaryAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    // Optional tags:
    // AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.PrimaryAttack")));
    // ActivationBlockedTags.AddTag(...);
}

void UGA_PrimaryAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    AlreadyHit.Reset();

    // 1) Wait for the Gameplay Event "Event.Melee.Hit"
    const FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hit"));
    UAbilityTask_WaitGameplayEvent* WaitHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitTag, nullptr, false, false);
    if (WaitHit)
    {
        WaitHit->EventReceived.AddDynamic(this, &UGA_PrimaryAttack::OnHitEventReceived);
        WaitHit->ReadyForActivation();
    }

    // 2) Play montage
    if (AttackMontage && ActorInfo && ActorInfo->GetAnimInstance())
    {
        UAbilityTask_PlayMontageAndWait* Play = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, AttackMontage, 1.f, NAME_None, false, 1.f, false, 1.f);
        Play->OnCompleted.AddDynamic(this, &UGA_PrimaryAttack::OnMontageCompleted);
        Play->OnInterrupted.AddDynamic(this, &UGA_PrimaryAttack::OnMontageInterrupted);
        Play->OnCancelled.AddDynamic(this, &UGA_PrimaryAttack::OnMontageCancelled);
        Play->ReadyForActivation();
    }
    else
    {
        // No montage? Just do a one-off hit immediately and end.
        DoHitSweepAndApply(ActorInfo);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UGA_PrimaryAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    AlreadyHit.Reset();
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_PrimaryAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_PrimaryAttack::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_PrimaryAttack::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_PrimaryAttack::OnHitEventReceived(FGameplayEventData Payload)
{
    DoHitSweepAndApply(CurrentActorInfo);
}

void UGA_PrimaryAttack::DoHitSweepAndApply(const FGameplayAbilityActorInfo* ActorInfo)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !DamageEffectClass) return;

    ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Char) return;

    USkeletalMeshComponent* Mesh = Char->GetMesh();
    const FTransform SockRight = Mesh && Mesh->DoesSocketExist(RightHandSocket) ? Mesh->GetSocketTransform(RightHandSocket) : Char->GetActorTransform();
    const FTransform SockLeft = Mesh && Mesh->DoesSocketExist(LeftHandSocket) ? Mesh->GetSocketTransform(LeftHandSocket) : Char->GetActorTransform();

    auto SweepOnce = [&](const FTransform& From)
        {
            UWorld* World = Char->GetWorld();
            if (!World) return;

            const FVector Start = From.GetLocation();
            const FVector Dir = From.GetRotation().GetForwardVector();
            const FVector End = Start + Dir * HitRange;

            FCollisionQueryParams Params(SCENE_QUERY_STAT(PrimaryAttackMelee), false, Char);
            TArray<FHitResult> Hits;
            World->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
                TraceChannel, FCollisionShape::MakeSphere(HitRadius), Params);

            for (const FHitResult& HR : Hits)
            {
                AActor* HitActor = HR.GetActor();
                if (!HitActor || HitActor == Char) continue;
                if (AlreadyHit.Contains(HitActor)) continue;

                // Get target ASC
                UAbilitySystemComponent* TargetASC =
                    UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
                if (!TargetASC) continue;

                // Make damage spec with SetByCaller = -Damage
                FGameplayEffectContextHandle Ctx = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
                Ctx.AddSourceObject(this);

                FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
                if (!Spec.IsValid()) continue;
                Spec.Data->SetContext(Ctx);

                const FGameplayTag DataDamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
                Spec.Data->SetSetByCallerMagnitude(DataDamageTag, -FMath::Max(0.f, Damage));

                GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

                AlreadyHit.Add(HitActor);
            }
        };

    // Try both hands (covers dual-wield or if one socket is missing)
    SweepOnce(SockRight);
    SweepOnce(SockLeft);
}
