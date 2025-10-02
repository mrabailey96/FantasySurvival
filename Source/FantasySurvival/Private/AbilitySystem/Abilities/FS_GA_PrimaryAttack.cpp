// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_PrimaryAttack.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Kismet/KismetSystemLibrary.h"
// #include "DrawDebugHelpers.h" // uncomment if you want debug shapes

UFS_GA_PrimaryAttack::UFS_GA_PrimaryAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Sensible default for the montage-event tag (override in defaults if you use another)
	if (!MeleeHitEventTag.IsValid())
	{
		MeleeHitEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Melee.Hit")));
	}
}

void UFS_GA_PrimaryAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 1) Commit or end
	if (!CommitOrEnd())
	{
		return;
	}

	AlreadyHit.Reset();

	// 2) Wait for the gameplay event (from the montage notify)
	if (MeleeHitEventTag.IsValid())
	{
		if (UAbilityTask_WaitGameplayEvent* WaitHit = WaitForEventTag(MeleeHitEventTag, /*OnlyOnce*/false, /*Exact*/false))
		{
			WaitHit->EventReceived.AddDynamic(this, &UFS_GA_PrimaryAttack::OnHitEventReceived);
			WaitHit->ReadyForActivation();
		}
	}

	// 3) Play the montage through the base helper (sets up delegates)
	BeginAbilityMontage(AttackMontage);
}

void UFS_GA_PrimaryAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	if (!Info || !Info->AvatarActor.IsValid()) return;

	// Damage application must be server-authoritative
	if (!Info->AvatarActor->HasAuthority())
	{
		return;
	}

	DoHitSweepAndApply(Info);
}

void UFS_GA_PrimaryAttack::OnMontageCompleted()
{
	// When montage finishes normally, end the ability. (Interrupt/Cancel paths are handled in base.)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}

void UFS_GA_PrimaryAttack::DoHitSweepAndApply(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !DamageGameplayEffect) return;

	AActor* Avatar = ActorInfo->AvatarActor.Get();
	ACharacter* Char = Cast<ACharacter>(Avatar);
	USkeletalMeshComponent* Mesh = Char ? Char->GetMesh() : nullptr;

	auto DoOneSweep = [&](const FName& SocketName)
		{
			FVector Start = Avatar->GetActorLocation();
			FRotator Facing = Avatar->GetActorRotation();

			if (Mesh && Mesh->DoesSocketExist(SocketName))
			{
				Start = Mesh->GetSocketLocation(SocketName);
				Facing = Mesh->GetSocketRotation(SocketName);
			}

			const FVector End = Start + (Facing.Vector() * HitRange);

			TArray<FHitResult> Hits;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(WarriorPrimaryAttack), /*bTraceComplex=*/false, Avatar);
			Params.bReturnPhysicalMaterial = false;

			const bool bAny = Avatar->GetWorld()->SweepMultiByChannel(
				Hits, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(HitRadius), Params);

			 // Debug
			 DrawDebugSphere(Avatar->GetWorld(), Start, HitRadius, 16, FColor::White, false, 1.f);
			 DrawDebugSphere(Avatar->GetWorld(), End,   HitRadius, 16, FColor::White, false, 1.f);
			 DrawDebugLine(Avatar->GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 1.f);

			if (!bAny) return;

			// Prepare source spec context once
			UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
			if (!SourceASC) return;

			FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
			Ctx.AddSourceObject(Avatar);

			for (const FHitResult& H : Hits)
			{
				AActor* Target = H.GetActor();
				if (!Target || Target == Avatar) continue;

				// One hit per unique target per attack window
				if (AlreadyHit.Contains(Target)) continue;

				// Target must have an ASC
				UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
				if (!TargetASC) continue;

				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect, GetAbilityLevel(), Ctx);
				if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid()) continue;

				// SetByCaller "Data.Damage" — pass negative so Additive to Health subtracts
				const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Data.Damage")));
				SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, -FMath::Abs(DamageAmount));

				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				AlreadyHit.Add(Target);
			}
		};

	// Right hand
	DoOneSweep(RightHandSocket);

	// Optional offhand
	if (bSweepLeftHandToo)
	{
		DoOneSweep(LeftHandSocket);
	}
}