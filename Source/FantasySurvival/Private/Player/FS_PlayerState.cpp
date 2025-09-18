// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/FS_PlayerState.h"
#include "AbilitySystem/FS_AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "AbilitySystem/FS_ClassConfig.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"

AFS_PlayerState::AFS_PlayerState()
{
    SetNetUpdateFrequency(60.f);

    AbilitySystemComponent = CreateDefaultSubobject<UFS_AbilitySystemComponent>(TEXT("ASC"));
    AttributeSetStats = CreateDefaultSubobject<UFS_AttributeSet_Stats>(TEXT("AttributeSet_Stats"));

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetIsReplicated(true);
        AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    }
}

UAbilitySystemComponent* AFS_PlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AFS_PlayerState::InitializeAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
    if (!AbilitySystemComponent) return;
    AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, InAvatarActor);
}

void AFS_PlayerState::ApplyClassInitialization()
{
    if (!AbilitySystemComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("FS_PlayerState: ASC is null; cannot apply class init."));
        return;
    }
    if (bInitApplied && bAbilitiesGranted)
    {
        return; // everything already applied
    }
    if (!ClassConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("FS_PlayerState: ClassConfig is null; assign DA_ClassConfig in defaults."));
        return;
    }

    // --- Instant Init (stats) ---
    if (!bInitApplied)
    {
        if (const TSubclassOf<UGameplayEffect>* FoundInit = ClassConfig->ClassInitEffects.Find(SelectedClass))
        {
            if (FoundInit && *FoundInit)
            {
                FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
                Ctx.AddSourceObject(this);

                FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(*FoundInit, 1.f, Ctx);
                if (Spec.IsValid())
                {
                    AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                }
            }
        }

        // Optional passive (infinite)
        if (const TSubclassOf<UGameplayEffect>* FoundPassive = ClassConfig->ClassPassiveEffects.Find(SelectedClass))
        {
            if (FoundPassive && *FoundPassive)
            {
                FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
                Ctx.AddSourceObject(this);

                FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(*FoundPassive, 1.f, Ctx);
                if (Spec.IsValid())
                {
                    AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                }
            }
        }

        // Snap current pools to max (start full)
        if (AttributeSetStats)
        {
            const float NewMaxHealth = AttributeSetStats->GetMaxHealth();
            const float NewMaxMana = AttributeSetStats->GetMaxMana();
            const float NewMaxStamina = AttributeSetStats->GetMaxStamina();

            AbilitySystemComponent->SetNumericAttributeBase(
                UFS_AttributeSet_Stats::GetHealthAttribute(), NewMaxHealth);
            AbilitySystemComponent->SetNumericAttributeBase(
                UFS_AttributeSet_Stats::GetManaAttribute(), NewMaxMana);
            AbilitySystemComponent->SetNumericAttributeBase(
                UFS_AttributeSet_Stats::GetStaminaAttribute(), NewMaxStamina);
        }

        bInitApplied = true;
    }

    // --- Grant default abilities for this class (server-only) ---
    if (HasAuthority() && !bAbilitiesGranted && ClassConfig && AbilitySystemComponent)
    {
        if (const FFS_ClassGrants* Grants = ClassConfig->ClassDefaultGrants.Find(SelectedClass))
        {
            for (const FFS_AbilityGrant& Grant : Grants->Abilities)
            {
                if (!Grant.Ability) continue;

                const int32 Level = FMath::Max(1, Grant.Level);
                // If designer left InputID at 0, default to Ability1
                const int32 InputID = (Grant.InputID != 0) ? Grant.InputID
                    : static_cast<int32>(EFSAbilityInputID::Ability1);

                FGameplayAbilitySpec Spec(Grant.Ability, Level, InputID, this);
                AbilitySystemComponent->GiveAbility(Spec);
            }
            bAbilitiesGranted = true;
        }
    }
}

void AFS_PlayerState::OnRep_SelectedClass()
{
    // Client UI hook if needed (don’t apply effects here).
}

void AFS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFS_PlayerState, SelectedClass);
}