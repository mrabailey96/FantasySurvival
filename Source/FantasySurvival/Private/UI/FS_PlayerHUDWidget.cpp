// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FS_PlayerHUDWidget.h"
#include "AbilitySystem/FS_NativeTags.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemStats.h"
#include "GameplayEffectTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Player/FS_PlayerState.h"

void UFS_PlayerHUDWidget::InitializeFromPlayerState(AFS_PlayerState* InPlayerState)
{
    if (!InPlayerState) return;

    ASC = InPlayerState->GetAbilitySystemComponent();
    Stats = InPlayerState->GetStatsSet();

    BindAttributeDelegates();
    BindStateTagEvents();
    RefreshAll();
    StartCooldownTicker();
}

void UFS_PlayerHUDWidget::NativeDestruct()
{
    StopCooldownTicker();
    Super::NativeDestruct();
}

void UFS_PlayerHUDWidget::BindAttributeDelegates()
{
    if (!ASC.IsValid() || !Stats.IsValid()) return;

    ASC->GetGameplayAttributeValueChangeDelegate(UFS_AttributeSet_Stats::GetHealthAttribute())
        .AddUObject(this, &UFS_PlayerHUDWidget::OnHealthChanged);
    ASC->GetGameplayAttributeValueChangeDelegate(UFS_AttributeSet_Stats::GetMaxHealthAttribute())
        .AddUObject(this, &UFS_PlayerHUDWidget::OnMaxHealthChanged);

    ASC->GetGameplayAttributeValueChangeDelegate(UFS_AttributeSet_Stats::GetManaAttribute())
        .AddUObject(this, &UFS_PlayerHUDWidget::OnManaChanged);
    ASC->GetGameplayAttributeValueChangeDelegate(UFS_AttributeSet_Stats::GetMaxManaAttribute())
        .AddUObject(this, &UFS_PlayerHUDWidget::OnMaxManaChanged);

    ASC->GetGameplayAttributeValueChangeDelegate(UFS_AttributeSet_Stats::GetStaminaAttribute())
        .AddUObject(this, &UFS_PlayerHUDWidget::OnStaminaChanged);
    ASC->GetGameplayAttributeValueChangeDelegate(UFS_AttributeSet_Stats::GetMaxStaminaAttribute())
        .AddUObject(this, &UFS_PlayerHUDWidget::OnMaxStaminaChanged);
}

void UFS_PlayerHUDWidget::RefreshAll()
{
    if (!ASC.IsValid() || !Stats.IsValid()) return;

    const float H = Stats->GetHealth();
    const float HMax = Stats->GetMaxHealth();
    if (HealthBar)  HealthBar->SetPercent(SafeDiv(H, HMax));
    if (HealthText) HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), H, HMax)));

    const float M = Stats->GetMana();
    const float MMax = Stats->GetMaxMana();
    if (ManaBar)  ManaBar->SetPercent(SafeDiv(M, MMax));
    if (ManaText) ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), M, MMax)));

    const float S = Stats->GetStamina();
    const float SMax = Stats->GetMaxStamina();
    if (StaminaBar)  StaminaBar->SetPercent(SafeDiv(S, SMax));
    if (StaminaText) StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), S, SMax)));
}

void UFS_PlayerHUDWidget::OnHealthChanged(const FOnAttributeChangeData&) { RefreshAll(); }
void UFS_PlayerHUDWidget::OnMaxHealthChanged(const FOnAttributeChangeData&) { RefreshAll(); }
void UFS_PlayerHUDWidget::OnManaChanged(const FOnAttributeChangeData&) { RefreshAll(); }
void UFS_PlayerHUDWidget::OnMaxManaChanged(const FOnAttributeChangeData&) { RefreshAll(); }
void UFS_PlayerHUDWidget::OnStaminaChanged(const FOnAttributeChangeData&) { RefreshAll(); }
void UFS_PlayerHUDWidget::OnMaxStaminaChanged(const FOnAttributeChangeData&) { RefreshAll(); }

void UFS_PlayerHUDWidget::StartCooldownTicker()
{
    if (!GetWorld()) return;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UFS_PlayerHUDWidget::TickCooldowns, 0.05f, true);
}
void UFS_PlayerHUDWidget::StopCooldownTicker()
{
    if (!GetWorld()) return;
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
}

void UFS_PlayerHUDWidget::TickCooldowns()
{
    if (!ASC.IsValid()) return;

    auto UpdateBar = [&](UProgressBar* Bar, const FGameplayTag& CooldownTag)
        {
            if (!Bar) return;

            // Find all active GEs that OWN this cooldown tag (e.g., Cooldown.PrimaryAttack)
            FGameplayTagContainer Tags; Tags.AddTag(CooldownTag);
            FGameplayEffectQuery   Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(Tags);
            TArray<FActiveGameplayEffectHandle> Handles = ASC->GetActiveEffects(Query);

            float RemainingBest = 0.f;
            float DurationBest = 0.f;
            bool  bFound = false;

            const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

            for (const FActiveGameplayEffectHandle& H : Handles)
            {
                if (const FActiveGameplayEffect* AGE = ASC->GetActiveGameplayEffect(H))
                {
                    // Duration < 0 means infinite; skip those for a "cooldown" display
                    const float Duration = AGE->GetDuration();
                    if (Duration <= 0.f) continue;

                    const float EndTime = AGE->GetEndTime();   // world time when it ends
                    const float Remaining = FMath::Max(0.f, EndTime - Now);

                    if (Remaining > RemainingBest)
                    {
                        RemainingBest = Remaining;
                        DurationBest = Duration;
                        bFound = true;
                    }
                }
            }

            if (!bFound || DurationBest <= 0.f)
            {
                Bar->SetPercent(1.f);
                Bar->SetRenderOpacity(0.f); // hide when not cooling
            }
            else
            {
                const float pct = 1.f - FMath::Clamp(RemainingBest / DurationBest, 0.f, 1.f);
                Bar->SetPercent(pct);
                Bar->SetRenderOpacity(1.f);
            }
        };

    UpdateBar(PrimaryAttackCooldown, TAG_Cooldown_PrimaryAttack);
    // UpdateBar(BlockCooldown, TAG_Cooldown_Block); // if/when you add one
}

void UFS_PlayerHUDWidget::BindStateTagEvents()
{
    if (!ASC.IsValid()) return;

    ASC->RegisterGameplayTagEvent(TAG_State_Blocking, EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &UFS_PlayerHUDWidget::OnStateTagChanged);
    ASC->RegisterGameplayTagEvent(TAG_State_Attacking, EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &UFS_PlayerHUDWidget::OnStateTagChanged);

    OnStateTagChanged(TAG_State_Blocking, ASC->GetGameplayTagCount(TAG_State_Blocking));
    OnStateTagChanged(TAG_State_Attacking, ASC->GetGameplayTagCount(TAG_State_Attacking));
}

void UFS_PlayerHUDWidget::OnStateTagChanged(FGameplayTag Tag, int32 NewCount)
{
    const bool bHas = (NewCount > 0);
    if (Tag == TAG_State_Blocking && BlockingIcon)
    {
        BlockingIcon->SetRenderOpacity(bHas ? 1.f : 0.f);
    }
    else if (Tag == TAG_State_Attacking && AttackingIcon)
    {
        AttackingIcon->SetRenderOpacity(bHas ? 1.f : 0.f);
    }
}
