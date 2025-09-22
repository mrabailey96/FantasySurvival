// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/FS_GA_ShadowStep.h"
#include "AbilitySystem/FS_NativeTags.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UFS_GA_ShadowStep::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitOrEnd()) return;

    if (UAbilityTask_WaitGameplayEvent* Wait = WaitForEventTag(TAG_Event_ShadowStep_Teleport, false, false))
    {
        Wait->EventReceived.AddDynamic(this, &UFS_GA_ShadowStep::OnTeleportEvent);
        Wait->ReadyForActivation();
    }

    if (!BeginAbilityMontage(ResolveMontage(ActorInfo), 1.f, NAME_None, false))
    {
        OnTeleportEvent(FGameplayEventData());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UFS_GA_ShadowStep::OnTeleportEvent(FGameplayEventData /*Payload*/)
{
    ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Char) return;

    const FVector Start = Char->GetActorLocation();
    const FVector End = Start + Char->GetActorForwardVector() * MaxRange;

    // Sweep to find a safe landing spot
    FHitResult Hit;
    TArray<AActor*> Ignored; Ignored.Add(Char);
    UKismetSystemLibrary::CapsuleTraceSingle(
        Char,
        Start, End,
        Char->GetCapsuleComponent()->GetScaledCapsuleRadius(),
        Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        /*bTraceComplex*/false, Ignored,
        EDrawDebugTrace::None, Hit, /*bIgnoreSelf*/true);

    FVector Target = End;
    if (Hit.bBlockingHit)
    {
        Target = Hit.Location - Char->GetActorForwardVector() * 50.f;
    }

    Char->TeleportTo(Target, Char->GetActorRotation(), false, true);
}