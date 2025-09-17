// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_Cleave.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UFS_GA_Cleave::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitOrEnd()) return;

	ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Char) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

	// Simple cone-ish melee: do a short forward sweep trace
	const FVector Start = Char->GetActorLocation() + Char->GetActorForwardVector() * 50.f;
	const FVector End = Start + Char->GetActorForwardVector() * 200.f;

	TArray<AActor*> Ignored; Ignored.Add(Char);
	FHitResult Hit;
	UKismetSystemLibrary::SphereTraceSingle(Char, Start, End, 60.f,
		UEngineTypes::ConvertToTraceType(ECC_Pawn), false, Ignored,
		EDrawDebugTrace::None, Hit, true);

	// TODO: Apply a damage GE to Hit.GetActor()

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
