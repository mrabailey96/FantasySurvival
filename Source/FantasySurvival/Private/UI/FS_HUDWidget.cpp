// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FS_HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "CommonActivatableWidget.h"

void UFS_HUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	// Keep it light; actual wiring is done in InitializeForASC
}

void UFS_HUDWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	SubscribeAttributeDelegates();
	RefreshBars();
}

void UFS_HUDWidget::NativeOnDeactivated()
{
	UnsubscribeAttributeDelegates();
	Super::NativeOnDeactivated();
}

void UFS_HUDWidget::InitializeForASC(UAbilitySystemComponent* InASC, FGameplayAttribute InHealthAttr, FGameplayAttribute InMaxHealthAttr,
	FGameplayAttribute InStaminaAttr, FGameplayAttribute InMaxStaminaAttr)
{
	ASC = InASC;
	HealthAttr = InHealthAttr;
	MaxHealthAttr = InMaxHealthAttr;
	StaminaAttr = InStaminaAttr;
	MaxStaminaAttr = InMaxStaminaAttr;

	if (IsActivated())
	{
		UnsubscribeAttributeDelegates();
		SubscribeAttributeDelegates();
		RefreshBars();
	}
}

void UFS_HUDWidget::SubscribeAttributeDelegates()
{
	if (!ASC) return;

	if (HealthAttr.IsValid())
	{
		HealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr)
			.AddUObject(this, &UFS_HUDWidget::OnHealthChanged);
	}
	if (MaxHealthAttr.IsValid())
	{
		MaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttr)
			.AddUObject(this, &UFS_HUDWidget::OnMaxHealthChanged);
	}
	if (StaminaAttr.IsValid())
	{
		StaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(StaminaAttr)
			.AddUObject(this, &UFS_HUDWidget::OnStaminaChanged);
	}
	if (MaxStaminaAttr.IsValid())
	{
		MaxStaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(MaxStaminaAttr)
			.AddUObject(this, &UFS_HUDWidget::OnMaxStaminaChanged);
	}
}

void UFS_HUDWidget::UnsubscribeAttributeDelegates()
{
	if (!ASC) return;

	if (HealthChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr).Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}
	if (MaxHealthChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttr).Remove(MaxHealthChangedHandle);
		MaxHealthChangedHandle.Reset();
	}
	if (StaminaChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(StaminaAttr).Remove(StaminaChangedHandle);
		StaminaChangedHandle.Reset();
	}
	if (MaxStaminaChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(MaxStaminaAttr).Remove(MaxStaminaChangedHandle);
		MaxStaminaChangedHandle.Reset();
	}
}

void UFS_HUDWidget::RefreshBars()
{
	if (!ASC) return;

	const float CurHealth = HealthAttr.IsValid() ? ASC->GetNumericAttribute(HealthAttr) : 0.f;
	const float MaxHealth = MaxHealthAttr.IsValid() ? ASC->GetNumericAttribute(MaxHealthAttr) : 1.f;
	const float CurStamina = StaminaAttr.IsValid() ? ASC->GetNumericAttribute(StaminaAttr) : 0.f;
	const float MaxStamina = MaxStaminaAttr.IsValid() ? ASC->GetNumericAttribute(MaxStaminaAttr) : 1.f;

	SetHealth(CurHealth, MaxHealth);
	SetStamina(CurStamina, MaxStamina);
}

void UFS_HUDWidget::OnHealthChanged(const FOnAttributeChangeData& Data) { RefreshBars(); }
void UFS_HUDWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data) { RefreshBars(); }
void UFS_HUDWidget::OnStaminaChanged(const FOnAttributeChangeData& Data) { RefreshBars(); }
void UFS_HUDWidget::OnMaxStaminaChanged(const FOnAttributeChangeData& Data) { RefreshBars(); }

void UFS_HUDWidget::SetHealth(float Current, float Max)
{
	if (HealthBar)
	{
		const float Denom = FMath::Max(Max, KINDA_SMALL_NUMBER);
		HealthBar->SetPercent(FMath::Clamp(Current / Denom, 0.f, 1.f));
	}
	if (HealthText)
	{
		HealthText->SetText(FText::AsNumber(FMath::Max(0, FMath::RoundToInt(Current))));
	}
}

void UFS_HUDWidget::SetStamina(float Current, float Max)
{
	if (StaminaBar)
	{
		const float Denom = FMath::Max(Max, KINDA_SMALL_NUMBER);
		StaminaBar->SetPercent(FMath::Clamp(Current / Denom, 0.f, 1.f));
	}
	if (StaminaText)
	{
		StaminaText->SetText(FText::AsNumber(FMath::Max(0, FMath::RoundToInt(Current))));
	}
}
