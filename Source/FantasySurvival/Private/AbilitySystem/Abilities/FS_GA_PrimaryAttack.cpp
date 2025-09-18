// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_PrimaryAttack.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "Player/FS_PlayerState.h"
#include "Characters/FS_PlayerClass.h" // your enum for classes

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayEffect.h"
#include "DrawDebugHelpers.h"

static FGameplayTag TAG_Ability_PrimaryAttack = FGameplayTag::RequestGameplayTag(TEXT("Ability.PrimaryAttack"));
static FGameplayTag TAG_Event_MeleeHitWindow = FGameplayTag::RequestGameplayTag(TEXT("Event.MeleeHitWindow"));
static FGameplayTag TAG_Cooldown_PrimaryAttack = FGameplayTag::RequestGameplayTag(TEXT("Cooldown.PrimaryAttack"));

UFS_GA_PrimaryAttack::UFS_GA_PrimaryAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    FGameplayTagContainer DefaultTags; DefaultTags.AddTag(TAG_Ability_PrimaryAttack);
    SetAssetTags(DefaultTags);
    ActivationOwnedTags.AddTag(TAG_Ability_PrimaryAttack);
}

UAnimMontage* UFS_GA_PrimaryAttack::ResolveClassMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (!ActorInfo) return nullptr;
    const AFS_PlayerState* PS = Cast<AFS_PlayerState>(ActorInfo->OwnerActor.Get());
    if (!PS) return nullptr;

    switch (PS->SelectedClass)
    {
    case EFSPlayerClass::Warrior:  return WarriorMontage;
    case EFSPlayerClass::Mage:     return MageMontage;
    case EFSPlayerClass::Assassin: return AssassinMontage;
    case EFSPlayerClass::Ranger:   return RangerMontage;
    default: return WarriorMontage;
    }
}

void UFS_GA_PrimaryAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    bConsumedHitWindow = false;

    ACharacter* Character = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    UAnimMontage* AttackMontage = ResolveClassMontage(ActorInfo);
    if (!Character || !AttackMontage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (UAbilityTask_WaitGameplayEvent* WaitEvent =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_MeleeHitWindow, nullptr, false, false))
    {
        WaitEvent->EventReceived.AddDynamic(this, &UFS_GA_PrimaryAttack::OnHitWindowEvent);
        WaitEvent->ReadyForActivation();
    }

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, AttackMontage, 1.f, NAME_None, false);

    if (MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &UFS_GA_PrimaryAttack::OnMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &UFS_GA_PrimaryAttack::OnMontageCompleted);
        MontageTask->OnCancelled.AddDynamic(this, &UFS_GA_PrimaryAttack::OnMontageCompleted);
        MontageTask->ReadyForActivation();
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UFS_GA_PrimaryAttack::OnHitWindowEvent(FGameplayEventData Payload)
{
    if (bConsumedHitWindow) return;
    bConsumedHitWindow = true;

    const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
    ACharacter* Character = Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;
    if (!Character || !DamageEffectClass) return;

    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (!Mesh) return;

    const FName Socket = TraceSocketName.IsNone() ? FName(TEXT("hand_rSocket")) : TraceSocketName;
    const FVector Start = Mesh->GetSocketLocation(Socket);
    const FVector Dir = Character->GetActorForwardVector();
    const FVector End = Start + Dir * TraceRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PrimaryAttackSweep), false, Character);
    FCollisionResponseParams Resp;

    if (UWorld* World = Character->GetWorld())
    {
        if (World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeSphere(TraceRadius), Params, Resp))
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (HitActor != Character)
                {
                    if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor))
                    {
                        if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
                        {
                            FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
                            Ctx.AddSourceObject(this);
                            Ctx.AddInstigator(Character, Character->GetController());

                            FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Ctx);

                            // TODO (later): insert per-weapon damage via SetByCaller here

                            if (Spec.IsValid())
                            {
                                SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
                            }
                        }
                    }
                }
            }
        }

#if !(UE_BUILD_SHIPPING)
        DrawDebugLine(World, Start, End, FColor::Silver, false, 1.5f, 0, 0.75f);
        DrawDebugSphere(World, Hit.bBlockingHit ? Hit.ImpactPoint : End, TraceRadius, 12,
            Hit.bBlockingHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 1.5f);
#endif
    }
}

void UFS_GA_PrimaryAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UFS_GA_PrimaryAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}