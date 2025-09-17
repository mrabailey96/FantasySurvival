// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_ShadowStep.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CapsuleComponent.h"

void UFS_GA_ShadowStep::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitOrEnd()) return;

	ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Char) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

	const float MaxRange = 600.f;
	const FVector Start = Char->GetActorLocation();
	const FVector End = Start + Char->GetActorForwardVector() * MaxRange;

	TArray<AActor*> Ignored; Ignored.Add(Char);
	FHitResult Hit;
	UKismetSystemLibrary::CapsuleTraceSingle(Char, Start, End,
		Char->GetCapsuleComponent()->GetScaledCapsuleRadius(),
		Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Ignored,
		EDrawDebugTrace::None, Hit, true);

	FVector Target = End;
	if (Hit.bBlockingHit) // stop just before obstacle
	{
		Target = Hit.Location - Char->GetActorForwardVector() * 50.f;
	}

	Char->TeleportTo(Target, Char->GetActorRotation(), false, true);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
