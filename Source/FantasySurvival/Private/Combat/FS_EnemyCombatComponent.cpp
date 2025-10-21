// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/FS_EnemyCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Animations/FS_EnemyAnimInstance.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UFS_EnemyCombatComponent::UFS_EnemyCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetByCallerDamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage")); // Adjust if changed
}

// Called when the game starts
void UFS_EnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find owners ASC once
	if (AActor* Owner = GetOwner())
	{
		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	}
	
}

bool UFS_EnemyCombatComponent::TryAttack(AActor* Target)
{
	if (!Target || !GetOwner() || !ASC || !DamageEffectClass) return false;

	// Optional: Server authority gate - montage can still play client side for visuals
	if (!GetOwner()->HasAuthority()) return false;

	const double Now = FPlatformTime::Seconds();
	if (Now - LastAttackTime < AttackCooldown) return false;

	const FVector From = GetOwner()->GetActorLocation();
	const FVector To = Target->GetActorLocation();
	if (!IsWithinRangeSq(From, To, AttackRange)) return false;

	// Play attack montage (no damage here)
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UAnimInstance* Anim = OwnerChar->GetMesh() ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr)
		{
			if (UFS_EnemyAnimInstance* EnemyAnim = Cast<UFS_EnemyAnimInstance>(Anim))
			{
				if (EnemyAnim->MeleeMontage)
				{
					AlreadyHitThisWindow.Reset();
					Anim->Montage_Play(EnemyAnim->MeleeMontage, 1.0f);

					// Also bind end/blend-out to be extra safe
					FOnMontageBlendingOutStarted BlendOut;
					BlendOut.BindUObject(this, &UFS_EnemyCombatComponent::OnAttackMontageBlendOut);
					Anim->Montage_SetBlendingOutDelegate(BlendOut, EnemyAnim->MeleeMontage);

					FOnMontageEnded Ended;
					Ended.BindUObject(this, &UFS_EnemyCombatComponent::OnAttackMontageEnded);
					Anim->Montage_SetEndDelegate(Ended, EnemyAnim->MeleeMontage);

					//Anim->Montage_Play(EnemyAnim->MeleeMontage, 1.0f);
				}
			}
		}
	}

	// Start cooldown now (locks in attempt)
	LastAttackTime = Now;

	return true;
}

bool UFS_EnemyCombatComponent::PerformMelee(AActor* Target)
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return false;

	// Build a short forward sweep (Centered near chest)
	const FVector Start = OwnerChar->GetActorLocation() + FVector(0, 0, 50);
	const FVector End = Start + OwnerChar->GetActorForwardVector() * MeleeTraceForward;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(MeleeTraceRadius);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FS_EnemyMelee), false);
	Params.AddIgnoredActor(OwnerChar);

	TArray<FHitResult> Hits;
	const bool bHit = OwnerChar->GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

	if (!bHit) return false;

	AActor* SourceInstigator = OwnerChar->GetInstigator() ? OwnerChar->GetInstigator() : OwnerChar;
	AActor* SourceAvatar = OwnerChar;

	for (const FHitResult& HR : Hits)
	{
		AActor* HitActor = HR.GetActor();
		if (!IsValid(HitActor) || HitActor == SourceAvatar) continue;

		// Per-Swing dedupe
		if (AlreadyHitThisWindow.Contains(HitActor)) continue;
		AlreadyHitThisWindow.Add(HitActor);

		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
		if (!TargetASC) continue;

		// Friendly fire guard: only damage players, not enemies
		static const FGameplayTag TAG_Team_Player = FGameplayTag::RequestGameplayTag(TEXT("Team.Player"));
		if (!TargetASC->HasMatchingGameplayTag(TAG_Team_Player))
		{
			// Not a player (e.g. another enemy) -> skip
			continue;
		}

		// Fresh context per target (prevents AddHitResult assertion)
		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		Ctx.AddSourceObject(this);
		Ctx.AddInstigator(SourceInstigator, SourceAvatar);
		Ctx.AddHitResult(HR);

		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Ctx);
		if (!Spec.IsValid()) continue;

		const float OutgoingDamage = -FMath::Abs(DamageBase); // subtract Health
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, SetByCallerDamageTag, OutgoingDamage);

		ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

		// Optional cues
		if (DamageNumberCueTag.IsValid())
		{
			FGameplayCueParameters NumParams;
			NumParams.Location = HR.ImpactPoint;
			NumParams.RawMagnitude = FMath::Abs(OutgoingDamage);
			NumParams.EffectContext = Ctx;
			TargetASC->ExecuteGameplayCue(DamageNumberCueTag, NumParams);
		}

		if (HitReactCueTag.IsValid())
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = HR.ImpactPoint;
			CueParams.Normal = HR.ImpactNormal;
			CueParams.EffectContext = Ctx;
			TargetASC->ExecuteGameplayCue(HitReactCueTag, CueParams);
		}
	}

	return true;
}

bool UFS_EnemyCombatComponent::IsWithinRangeSq(const FVector& From, const FVector& To, float Range)
{
	return FVector::DistSquared(From, To) <= FMath::Square(Range);
}

void UFS_EnemyCombatComponent::OnAttackMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	AlreadyHitThisWindow.Reset();
}

void UFS_EnemyCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	AlreadyHitThisWindow.Reset();
}

void UFS_EnemyCombatComponent::BeginMeleeWindow()
{
	AlreadyHitThisWindow.Reset();
}

void UFS_EnemyCombatComponent::EndMeleeWindow()
{
	// Optional cleanup; Currently nothing needed
}

void UFS_EnemyCombatComponent::PerformMeleeWindow()
{
	// Server only safety; Montage notifies can fire on all clients
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// If we want cooldown to begin on impact instad of animation start
	LastAttackTime = FPlatformTime::Seconds();

	// Reuse our existing sweep/apply (We'll dedupe inside PerformMelee by checking AlreadyHitThisWindow)
	PerformMelee(nullptr);
}