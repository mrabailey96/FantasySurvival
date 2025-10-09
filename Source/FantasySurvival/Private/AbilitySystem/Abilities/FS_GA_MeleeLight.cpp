// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_MeleeLight.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h" // Optional - for RequestGameplayTag
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemGlobals.h"
// AbilityTasks
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UFS_GA_MeleeLight::UFS_GA_MeleeLight()
{
	// Ability Instancing Policy: Owner actor (each avatar has its own instance)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Tag the ability so we can activate it by tag from input
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Melee.Light")));

	// WHile this ability is active, the owner has State.Attacking (used to pause regen)
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));

	// Default hit event tag if designer forgets to set it
	MeleeHitEventTag = FGameplayTag::RequestGameplayTag(FName("Event.MeleeHitWindow"));
}

void UFS_GA_MeleeLight::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) // handles costs/cooldown if assigned
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* AvatarChar = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!AvatarChar || !AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HitActorsThisActivation.Reset();

	// Play Montage
	if (UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayMeleeLight"), AttackMontage, 1.0f, NAME_None))
	{
		PlayTask->OnBlendOut.AddDynamic(this, &UFS_GA_MeleeLight::OnMontageFinished);
		PlayTask->OnCompleted.AddDynamic(this, &UFS_GA_MeleeLight::OnMontageFinished);
		PlayTask->OnInterrupted.AddDynamic(this, &UFS_GA_MeleeLight::OnMontageFinished);
		PlayTask->OnCancelled.AddDynamic(this, &UFS_GA_MeleeLight::OnMontageFinished);
		PlayTask->ReadyForActivation();
	}

	//Wait for the gameplay event "Event.MeleeHitWindow" sent from the montage notify
	if (UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MeleeHitEventTag, nullptr, false, true))
	{
		EventTask->EventReceived.AddDynamic(this, &UFS_GA_MeleeLight::OnMeleeHitEvent);
		EventTask->ReadyForActivation();
	}
	UE_LOG(LogTemp, Warning, TEXT("[MeleeLight] Activate: Waiting for %s"), *MeleeHitEventTag.ToString());
}

void UFS_GA_MeleeLight::OnMeleeHitEvent(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("MeleeLight] OnMeleeHitEvent fired"));
	// Perform the hit logic right when the notify fires
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	if (Info)
	{
		DoMeleeHit(Info);
	}
}

void UFS_GA_MeleeLight::DoMeleeHit(const FGameplayAbilityActorInfo* ActorInfo)
{
	ACharacter* AvatarChar = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!AvatarChar || !DamageEffectClass) return;

	// Build a simple forward sphere trace from the characters center
	const FVector Start = AvatarChar->GetActorLocation() + FVector(0, 0, 50);
	const FVector End = Start + AvatarChar->GetActorForwardVector() * TraceRange;

#if !(UE_BUILD_SHIPPING)
	if (bDebugTrace)
	{
		// Pre-trace visualization
		DrawDebugSphere(AvatarChar->GetWorld(), Start, TraceRadius, 16, FColor::Yellow, false, 1.5f, 0, 1.5f);
		DrawDebugSphere(AvatarChar->GetWorld(), End, TraceRadius, 16, FColor::Yellow, false, 1.5f, 0, 1.5f);
		DrawDebugLine(AvatarChar->GetWorld(), Start, End, FColor::Yellow, false, 1.5f, 0, 1.5f);
	}
#endif

	TArray<FHitResult> Hits;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(TraceRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GA_MeleeLight), false);
	Params.AddIgnoredActor(AvatarChar);

	const bool bHit = AvatarChar->GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

#if !(UE_BUILD_SHIPPING)
	if (bDebugTrace)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, bHit ? FColor::Red : FColor::Green, FString::Printf(TEXT("Melee sweep: %d hit(s)"), Hits.Num()));
		}
		for (const FHitResult& HR : Hits)
		{
			DrawDebugPoint(AvatarChar->GetWorld(), HR.ImpactPoint, 12.0f, FColor::Red, false, 1.5f);
		}
	}
#endif

	if (!bHit) return;

	// Prepare the GE spec we will apply to each hit target with SetByCaller damage
	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC) return;

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this); // optional

	const FGameplayTag DamageDataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));

	for (const FHitResult& HR : Hits)
	{
		AActor* HitActor = HR.GetActor();
		if (!HitActor || HitActorsThisActivation.Contains(HitActor)) continue;

		if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor))
		{
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				const float OutgoingDamage = -FMath::Abs(DamageBase); // ensure negative for damage
				SpecHandle.Data->SetSetByCallerMagnitude(DamageDataTag, OutgoingDamage);
				SpecHandle.Data->GetContext().AddHitResult(HR, false);

				auto Handle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

				UE_LOG(LogTemp, Warning, TEXT("[MeleeLight] Applied %0.1f to %s | HandleValid=%d"), OutgoingDamage, *GetNameSafe(HitActor), Handle.IsValid());

				HitActorsThisActivation.Add(HitActor);

				// OutgoingDamage is negative in our code; We want a positive number for the UI
				const float ShownAmount = FMath::Abs(OutgoingDamage);

				// Execute the damage number cue on the target so it replicates appropriately
				if (DamageNumberCueTag.IsValid())
				{
					FGameplayCueParameters NumParams;
					NumParams.Location = HR.ImpactPoint; // Falls back to actor location in the cue if needed
					NumParams.RawMagnitude = ShownAmount; // The number the cue will display
					NumParams.EffectContext = Context; // optional

					TargetASC->ExecuteGameplayCue(DamageNumberCueTag, NumParams);
				}
			}

			// Gate by cooldown tag on the target
			const FGameplayTag HitReactCooldownTag = FGameplayTag::RequestGameplayTag(FName("Cooldown.HitReact"));

			if (!TargetASC->HasMatchingGameplayTag(HitReactCooldownTag))
			{
				// Apply the cooldown GE (authoritive)
				if (HitReactCooldownEffectClass)
				{
					FGameplayEffectContextHandle ContextCooldown = SourceASC->MakeEffectContext();
					FGameplayEffectSpecHandle SpecCD = SourceASC->MakeOutgoingSpec(HitReactCooldownEffectClass, 1.0f, ContextCooldown);
					if (SpecCD.IsValid())
					{
						SourceASC->ApplyGameplayEffectSpecToTarget(*SpecCD.Data.Get(), TargetASC);
					}
				}

				// Execute the hit-react gameplay cue with hit data
				if (HitReactCueTag.IsValid())
				{
					FGameplayCueParameters CueParams;
					CueParams.Location = HR.ImpactPoint;
					CueParams.Normal = HR.ImpactNormal;
					CueParams.PhysicalMaterial = HR.PhysMaterial.Get();
					CueParams.EffectContext = Context; //SpecHandle.Data->GetContext(); // reuse if we still have it in scope

					// Fire on the targets ASC so it replicates properly
					TargetASC->ExecuteGameplayCue(HitReactCueTag, CueParams);
				}
			}
		}
	}
}

void UFS_GA_MeleeLight::OnMontageFinished()
{
	// End when montage stops for any reason
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UFS_GA_MeleeLight::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	HitActorsThisActivation.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UFS_GA_MeleeLight::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Keep parent checks (cooldowns etc.)
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	// If no cost set, it's free
	if (StaminaCost <= 0.0f || !ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return true;
	}

	// Read current Stamina
	const float CurrentStamina = ActorInfo->AbilitySystemComponent->GetNumericAttribute(UFS_AttributeSet_Stats::GetStaminaAttribute());

	const bool bEnough = (CurrentStamina >= StaminaCost);
	if (!bEnough && OptionalRelevantTags)
	{
		// Optional: broadcast a fail tag we can listen for (UI beep, etc.)
		OptionalRelevantTags->AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Fail.Stamina")));
	}
	return bEnough;
}

void UFS_GA_MeleeLight::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid() || StaminaCost <= 0.0f || !StaminaCostEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(StaminaCostEffectClass, 1.0f, Context);
	if (Spec.IsValid())
	{
		// SetByCaller negative to Subtract stamina
		const FGameplayTag CostTag = FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost"));
		Spec.Data->SetSetByCallerMagnitude(CostTag, -FMath::Abs(StaminaCost));

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}
