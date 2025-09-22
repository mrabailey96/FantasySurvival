// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_PrimaryAttack.h"
#include "Player/FS_PlayerState.h"
#include "Characters/FS_PlayerClass.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayEffect.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/FS_NativeTags.h"

UFS_GA_PrimaryAttack::UFS_GA_PrimaryAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    // Asset Tag Metadata
    FGameplayTagContainer DefaultTags;
    DefaultTags.AddTag(TAG_Ability_PrimaryAttack);
    SetAssetTags(DefaultTags);

    // Tags applied to the owner while this ability is active
    ActivationOwnedTags.AddTag(TAG_Ability_PrimaryAttack);
    ActivationOwnedTags.AddTag(TAG_State_Attacking); // Player owns "Attacking" while this runs

    // Tags that prevent this ability from starting
    ActivationBlockedTags.AddTag(TAG_State_Blocking); // Cant start if currently blocking
}

void UFS_GA_PrimaryAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitOrEnd()) return;

	// Explicitly apply cooldown GE (in addition to the Ability's internal cooldown system)
	if (CooldownEffectClass)
	{
		FGameplayEffectSpecHandle CD = MakeOutgoingGameplayEffectSpec(CooldownEffectClass, GetAbilityLevel());
		if (CD.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CD);
		}
	}

	// Wait for the hit-window event (child-specific logic)
	if (UAbilityTask_WaitGameplayEvent* WaitEvent = WaitForEventTag(TAG_Event_MeleeHitWindow, /*Once*/false, /*Exact*/false))
	{
		WaitEvent->EventReceived.AddDynamic(this ,&UFS_GA_PrimaryAttack::OnHitWindowEvent);
		WaitEvent->ReadyForActivation();
	}

	// Ask base to start the montage with default bindings (it will EndAbility on finish)
	UAnimMontage* AttackMontage = ResolveMontage(ActorInfo);
	if (!BeginAbilityMontage(AttackMontage, /*Rate*/1.0f, /*Section*/NAME_None, /*bStopWhenAbilityEnds*/false))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility*/true, /*bWasCancelled*/true);
		return;
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

UAnimMontage* UFS_GA_PrimaryAttack::ResolveMontage(const FGameplayAbilityActorInfo* ActorInfo) const
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