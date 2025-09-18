// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/FS_CharacterAnimInstance.h"
#include "Player/FS_PlayerState.h"
#include "Characters/FS_Character.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogFSAnim, Log, All);

void UFS_CharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    APawn* Pawn = TryGetPawnOwner();
    CachedCharacter = Cast<AFS_Character>(Pawn);

    UE_LOG(LogFSAnim, Warning, TEXT("[Anim] Init: Pawn=%s"), Pawn ? *Pawn->GetName() : TEXT("NULL"));

    RefreshClassFromOwner();
    SelectPrimaryAttackMontage();

    UE_LOG(LogFSAnim, Warning, TEXT("[Anim] Init: Class=%d Montage=%s"),
        (int32)ClassType,
        PrimaryAttackMontage ? *PrimaryAttackMontage->GetName() : TEXT("None"));
}

void UFS_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Retry once PS is valid / or if montage not selected yet
    if (!bTriedInitClass || PrimaryAttackMontage == nullptr)
    {
        RefreshClassFromOwner();
        SelectPrimaryAttackMontage();

        UE_LOG(LogFSAnim, Verbose, TEXT("[Anim] Update: refresh Class=%d Montage=%s"),
            (int32)ClassType,
            PrimaryAttackMontage ? *PrimaryAttackMontage->GetName() : TEXT("None"));
    }

    if (bWantsPrimaryAttack)
    {
        const bool bPlaying = (PrimaryAttackMontage && Montage_IsPlaying(PrimaryAttackMontage));
        UE_LOG(LogFSAnim, Warning, TEXT("[Anim] Tick Trigger: WantsPrimaryAttack=true; IsPlaying=%d; Montage=%s"),
            bPlaying ? 1 : 0,
            PrimaryAttackMontage ? *PrimaryAttackMontage->GetName() : TEXT("None"));

        if (PrimaryAttackMontage && !bPlaying)
        {
            Montage_Play(PrimaryAttackMontage, AttackPlayRate);
            UE_LOG(LogFSAnim, Warning, TEXT("[Anim] Montage_Play fired (Rate=%.2f)"), AttackPlayRate);
        }
        bWantsPrimaryAttack = false; // consume
    }
}

void UFS_CharacterAnimInstance::RefreshClassFromOwner()
{
    bTriedInitClass = true;

    AFS_Character* C = CachedCharacter.Get();
    if (!C)
    {
        if (APawn* P = TryGetPawnOwner())
        {
            CachedCharacter = Cast<AFS_Character>(P);
            C = CachedCharacter.Get();
        }
    }

    if (!C)
    {
        UE_LOG(LogFSAnim, Verbose, TEXT("[Anim] RefreshClass: Character NULL"));
        return;
    }

    APlayerState* PSBase = C->GetPlayerState();
    AFS_PlayerState* PS = Cast<AFS_PlayerState>(PSBase);
    if (PS)
    {
        ClassType = PS->SelectedClass;
        UE_LOG(LogFSAnim, Verbose, TEXT("[Anim] RefreshClass: SelectedClass=%d"), (int32)ClassType);
    }
    else
    {
        UE_LOG(LogFSAnim, Verbose, TEXT("[Anim] RefreshClass: PlayerState not ready"));
    }
}

void UFS_CharacterAnimInstance::SelectPrimaryAttackMontage()
{
    UAnimMontage* NewMontage = nullptr;

    switch (ClassType)
    {
    case EFSPlayerClass::Warrior:  NewMontage = PrimaryAttackAnimSet.WarriorMontage;  break;
    case EFSPlayerClass::Mage:     NewMontage = PrimaryAttackAnimSet.MageMontage;     break;
    case EFSPlayerClass::Ranger:   NewMontage = PrimaryAttackAnimSet.RangerMontage;   break;
    case EFSPlayerClass::Assassin: NewMontage = PrimaryAttackAnimSet.AssassinMontage; break;
    default: break;
    }

    if (PrimaryAttackMontage != NewMontage)
    {
        UE_LOG(LogFSAnim, Warning, TEXT("[Anim] SelectPrimaryAttackMontage: %s → %s"),
            PrimaryAttackMontage ? *PrimaryAttackMontage->GetName() : TEXT("None"),
            NewMontage ? *NewMontage->GetName() : TEXT("None"));
        PrimaryAttackMontage = NewMontage;
    }
}

void UFS_CharacterAnimInstance::StartPrimaryAttack(float InPlayRate)
{
    UE_LOG(LogFSAnim, Warning, TEXT("[Anim] StartPrimaryAttack(InPlayRate=%.2f) Montage=%s"),
        InPlayRate,
        PrimaryAttackMontage ? *PrimaryAttackMontage->GetName() : TEXT("None"));

    AttackPlayRate = InPlayRate;
    bWantsPrimaryAttack = (PrimaryAttackMontage != nullptr);
}
