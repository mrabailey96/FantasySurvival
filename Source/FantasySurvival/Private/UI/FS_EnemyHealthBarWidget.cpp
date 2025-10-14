// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FS_EnemyHealthBarWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"

void UFS_EnemyHealthBarWidget::InitializeFromASC(UAbilitySystemComponent* InASC)
{
	// Guard
	if (!InASC) return;

	ASC = InASC;

	// Cache attributes once
	HealthAttr = UFS_AttributeSet_Stats::GetHealthAttribute();
	MaxHealthAttr = UFS_AttributeSet_Stats::GetMaxHealthAttribute();

	// Initial fill
	const float CurrentHealth = ASC->GetNumericAttribute(HealthAttr);
	RefreshPercentFrom(CurrentHealth);

	// Subscribe to Health changes
	FOnGameplayAttributeValueChange& Delegate = ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr);
	HealthChangedHandle = Delegate.AddUObject(this, &UFS_EnemyHealthBarWidget::OnHealthChanged);
}

void UFS_EnemyHealthBarWidget::NativeDestruct()
{
	// Unbind cleanly
	if (ASC.IsValid() && HealthChangedHandle.IsValid())
	{
		FOnGameplayAttributeValueChange& Delegate = ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr);
		Delegate.Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}

	Super::NativeDestruct();
}

void UFS_EnemyHealthBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshPercentFrom(Data.NewValue);
}

void UFS_EnemyHealthBarWidget::RefreshPercentFrom(float CurrentHealth)
{
	if (!PB_Health) return;

	float MaxHealth = 1.0f;
	if (ASC.IsValid())
	{
		MaxHealth = ASC->GetNumericAttribute(MaxHealthAttr);
		if (MaxHealth <= 0.0f) MaxHealth = 1.0f; // Avoid div by 0
	}

	const float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
	PB_Health->SetPercent(Percent);
}

void UFS_EnemyHealthBarWidget::ApplyPolish(float Percent)
{
	// 0.1% tolerance so 0.999 counts as full
	const bool bIsFull = FMath::IsNearlyEqual(Percent, 1.0f, 0.001f);

	if (bHideWhenFull && bIsFull)
	{
		// Prefer hiding a named container if available
		if (RootContainer) RootContainer->SetVisibility(ESlateVisibility::Hidden);
		else SetVisibility(ESlateVisibility::Hidden); // Fallback: hide the whole widget

		// Extra belt and suspenders for WidgetComponent draws
		SetRenderOpacity(0.0f);
	}
	else
	{
		if (RootContainer) RootContainer->SetVisibility(ESlateVisibility::Visible);
		else SetVisibility(ESlateVisibility::Visible);

		SetRenderOpacity(1.0f);
	}
}