// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_PrimaryAttack.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "Player/FS_PlayerState.h"
#include "Characters/FS_PlayerClass.h"
#include "Characters/Animations/FS_CharacterAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameplayEffect.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFSAttack, Log, All);

// Gameplay Tags (make sure they exist in your Project Settings → Gameplay Tags)
static FGameplayTag TAG_Ability_PrimaryAttack = FGameplayTag::RequestGameplayTag(TEXT("Ability.PrimaryAttack"));
static FGameplayTag TAG_Event_MeleeHitWindow = FGameplayTag::RequestGameplayTag(TEXT("Event.MeleeHitWindow"));
static FGameplayTag TAG_Event_AttackEnd = FGameplayTag::RequestGameplayTag(TEXT("Event.Attack.End"));
static FGameplayTag TAG_Cooldown_PrimaryAttack = FGameplayTag::RequestGameplayTag(TEXT("Cooldown.PrimaryAttack"));

UFS_GA_PrimaryAttack::UFS_GA_PrimaryAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(TAG_Ability_PrimaryAttack);
    SetAssetTags(AssetTags);
    ActivationOwnedTags.AddTag(TAG_Ability_PrimaryAttack);
}

void UFS_GA_PrimaryAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] ActivateAbility"));

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogFSAttack, Warning, TEXT("[GA] CommitAbility = FALSE (blocked by cost/cooldown?)"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] CommitAbility = TRUE"));

    bConsumedHitWindow = false;

    // Wait for the melee hit-window (AnimNotify → Gameplay Event)
    if (UAbilityTask_WaitGameplayEvent* WaitHit =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_MeleeHitWindow, nullptr, false, false))
    {
        WaitHit->EventReceived.AddDynamic(this, &UFS_GA_PrimaryAttack::OnHitWindowEvent);
        WaitHit->ReadyForActivation();
        UE_LOG(LogFSAttack, Verbose, TEXT("[GA] Waiting for %s"), *TAG_Event_MeleeHitWindow.ToString());
    }

    // Wait for attack end (AnimNotify → Gameplay Event)
    if (UAbilityTask_WaitGameplayEvent* WaitEnd =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_AttackEnd, nullptr, false, false))
    {
        WaitEnd->EventReceived.AddDynamic(this, &UFS_GA_PrimaryAttack::OnAttackEndEvent);
        WaitEnd->ReadyForActivation();
        UE_LOG(LogFSAttack, Verbose, TEXT("[GA] Waiting for %s"), *TAG_Event_AttackEnd.ToString());
    }

    // Preferred path: call the AnimBP via our C++ AnimInstance parent
    ACharacter* Character = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;
    UAnimInstance* AnyAnim = Mesh ? Mesh->GetAnimInstance() : nullptr;

    UE_LOG(LogFSAttack, Warning, TEXT("[GA] AnimInstance on mesh = %s"),
        AnyAnim ? *AnyAnim->GetClass()->GetName() : TEXT("NULL"));

    if (UFS_CharacterAnimInstance* Anim = Mesh ? Cast<UFS_CharacterAnimInstance>(AnyAnim) : nullptr)
    {
        UE_LOG(LogFSAttack, Warning, TEXT("[GA] ABP path → StartPrimaryAttack(1.0)"));
        Anim->StartPrimaryAttack(1.f);
        // Ability ends when Event.Attack.End arrives
        return;
    }

    // Fallback: ability task directly plays a montage (in case ABP/parent not set)
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] ABP path unavailable → trying fallback montage"));
    UAnimMontage* AttackMontage = ResolveClassMontage(ActorInfo);
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] Fallback montage = %s"),
        AttackMontage ? *AttackMontage->GetName() : TEXT("NULL"));

    if (!Character || !AttackMontage)
    {
        UE_LOG(LogFSAttack, Warning, TEXT("[GA] Fallback failed → no Character or Montage"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, AttackMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds*/ false);

    if (MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &UFS_GA_PrimaryAttack::OnMontageCompleted);
        MontageTask->OnInterrupted.AddDynamic(this, &UFS_GA_PrimaryAttack::OnMontageCompleted);
        MontageTask->OnCancelled.AddDynamic(this, &UFS_GA_PrimaryAttack::OnMontageCompleted);
        MontageTask->ReadyForActivation();
        UE_LOG(LogFSAttack, Warning, TEXT("[GA] Fallback AbilityTask_PlayMontageAndWait started"));
    }
    else
    {
        UE_LOG(LogFSAttack, Warning, TEXT("[GA] Fallback AbilityTask_PlayMontageAndWait = NULL"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UFS_GA_PrimaryAttack::OnHitWindowEvent(FGameplayEventData /*Payload*/)
{
    if (bConsumedHitWindow) return;
    bConsumedHitWindow = true;

    UE_LOG(LogFSAttack, Warning, TEXT("[GA] >>> HitWindow EVENT received"));

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
        const bool bHit = World->SweepSingleByChannel(
            Hit, Start, End, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeSphere(TraceRadius), Params, Resp);

        UE_LOG(LogFSAttack, Warning, TEXT("[GA] Sweep: %s  Start=(%.1f,%.1f,%.1f) End=(%.1f,%.1f,%.1f)"),
            bHit ? TEXT("HIT") : TEXT("MISS"),
            Start.X, Start.Y, Start.Z, End.X, End.Y, End.Z);

#if !(UE_BUILD_SHIPPING)
        DrawDebugLine(World, Start, End, FColor::Silver, false, 1.5f, 0, 0.75f);
        DrawDebugSphere(World, bHit ? Hit.ImpactPoint : End, TraceRadius, 12,
            bHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 1.5f);
#endif
        if (bHit)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                UE_LOG(LogFSAttack, Warning, TEXT("[GA] Hit Actor: %s"), *HitActor->GetName());
                if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor))
                {
                    if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
                    {
                        FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
                        Ctx.AddSourceObject(this);
                        Ctx.AddInstigator(Character, Character->GetController());

                        FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Ctx);
                        if (Spec.IsValid())
                        {
                            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
                            UE_LOG(LogFSAttack, Warning, TEXT("[GA] Applied DamageEffect to %s"), *HitActor->GetName());
                        }
                    }
                }
            }
        }
    }
}

void UFS_GA_PrimaryAttack::OnAttackEndEvent(FGameplayEventData /*Payload*/)
{
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] <<< AttackEnd EVENT received → EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UFS_GA_PrimaryAttack::OnMontageCompleted()
{
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] Fallback montage finished → EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UFS_GA_PrimaryAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    UE_LOG(LogFSAttack, Warning, TEXT("[GA] EndAbility (Cancelled=%s)"),
        bWasCancelled ? TEXT("TRUE") : TEXT("FALSE"));

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UAnimMontage* UFS_GA_PrimaryAttack::ResolveClassMontage(const FGameplayAbilityActorInfo* ActorInfo) const
{
    const AFS_PlayerState* PS = ActorInfo ? Cast<AFS_PlayerState>(ActorInfo->OwnerActor.Get()) : nullptr;
    const EFSPlayerClass ClassType = PS ? PS->SelectedClass : EFSPlayerClass::Warrior;

    switch (ClassType)
    {
    case EFSPlayerClass::Warrior:  return WarriorMontage;
    case EFSPlayerClass::Mage:     return MageMontage;
    case EFSPlayerClass::Assassin: return AssassinMontage;
    case EFSPlayerClass::Ranger:   return RangerMontage;
    default:                       return WarriorMontage;
    }
}