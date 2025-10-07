// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FS_PlayerHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "GameplayEffectTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UFS_PlayerHUDWidget::InitializeFromASC(UAbilitySystemComponent* InASC, UFS_AttributeSet_Stats* InStats)
{
	ASC = InASC;
	Stats = InStats;

	if (!ASC || !Stats)
	{
		UE_LOG(LogTemp, Warning, TEXT("HUD Init Failed: ASC or Stats null"));
		return;
	}

	// --- Subscribe to attribute change delegates so the HUD updates live ---
	// Health + MaxHealth
	OnHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetHealthAttribute()).AddUObject(this, &UFS_PlayerHUDWidget::HandleHealthChanged);
	OnMaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetMaxHealthAttribute()).AddUObject(this, &UFS_PlayerHUDWidget::HandleHealthChanged);

	// Mana + MaxMana
	OnManaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetManaAttribute()).AddUObject(this, &UFS_PlayerHUDWidget::HandleManaChanged);
	OnMaxManaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetMaxManaAttribute()).AddUObject(this, &UFS_PlayerHUDWidget::HandleManaChanged);

	// Stamina + MaxStamina
	OnStaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetStaminaAttribute()).AddUObject(this, &UFS_PlayerHUDWidget::HandleStaminaChanged);
	OnMaxStaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetMaxStaminaAttribute()).AddUObject(this, &UFS_PlayerHUDWidget::HandleStaminaChanged);

	// Initial draw
	RefreshAll();
}

// Delegate wrappers (they receive the payload, we just refresh)
void UFS_PlayerHUDWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealth();
}

void UFS_PlayerHUDWidget::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	UpdateMana();
}

void UFS_PlayerHUDWidget::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStamina();
}

void UFS_PlayerHUDWidget::RefreshAll()
{
	UpdateHealth();
	UpdateMana();
	UpdateStamina();
}

void UFS_PlayerHUDWidget::UpdateHealth()
{
	if (!Stats) return;
	SetBarAndText(HealthBar, HealthText, Stats->GetHealth(), Stats->GetMaxHealth());
}

void UFS_PlayerHUDWidget::UpdateMana()
{
	if (!Stats) return;
	SetBarAndText(ManaBar, ManaText, Stats->GetMana(), Stats->GetMaxMana());
}

void UFS_PlayerHUDWidget::UpdateStamina()
{
	if (!Stats) return;
	SetBarAndText(StaminaBar, StaminaText, Stats->GetStamina(), Stats->GetMaxStamina());
}

void UFS_PlayerHUDWidget::SetBarAndText(UProgressBar* Bar, UTextBlock* Text, float Current, float Max)
{
	if (!Bar) return;

	const float SafeMax = FMath::Max(0.001f, Max); // Avoid divide by zero
	const float Percent = FMath::Clamp(Current / SafeMax, 0.0f, 1.0f);

	Bar->SetPercent(Percent);

	if (Text)
	{
		Text->SetText(FText::FromString(FString::Printf(TEXT("% .0f / % .0f"), Current, Max)));
	}
}

void UFS_PlayerHUDWidget::NativeDestruct()
{
	// Unbind delegates if ASC is valid
	if (ASC)
	{
		// Health + MaxHealth
		ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetHealthAttribute()).Remove(OnHealthChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetMaxHealthAttribute()).Remove(OnMaxHealthChangedHandle);

		// Mana + MaxMana
		ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetManaAttribute()).Remove(OnManaChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetMaxManaAttribute()).Remove(OnMaxManaChangedHandle);

		// Stamina + MaxStamina
		ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetStaminaAttribute()).Remove(OnStaminaChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(Stats->GetMaxStaminaAttribute()).Remove(OnMaxStaminaChangedHandle);
	}

	Super::NativeDestruct();
}
