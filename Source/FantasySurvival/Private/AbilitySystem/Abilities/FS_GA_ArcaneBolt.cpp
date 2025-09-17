// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_ArcaneBolt.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UFS_GA_ArcaneBolt::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitOrEnd()) return;
	ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Char) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

	const FVector Start = Char->GetActorLocation() + Char->GetControlRotation().Vector() * 50.f;
	const FVector End = Start + Char->GetControlRotation().Vector() * 1500.f;

	TArray<AActor*> Ignored; Ignored.Add(Char);
	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(Char, Start, End,
		UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Ignored,
		EDrawDebugTrace::None, Hit, true);

	// TODO: Spawn projectile VFX / apply damage GE to Hit actor
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
