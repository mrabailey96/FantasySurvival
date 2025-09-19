// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/FS_PlayerController.h"
#include "UI/FS_HUDWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"

AFS_PlayerController::AFS_PlayerController()
{
	bShowMouseCursor = false;
}

void AFS_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	EnsureHUD();
	WireASC();
}

void AFS_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	WireASC();
}

void AFS_PlayerController::EnsureHUD()
{
	if (HUDInstance || !HUDWidgetClass) return;

	HUDInstance = CreateWidget<UFS_HUDWidget>(this, HUDWidgetClass);
	if (HUDInstance)
	{
		HUDInstance->AddToViewport(/*ZOrder=*/0);
		HUDInstance->ActivateWidget();
	}
}

void AFS_PlayerController::WireASC()
{
    if (!HUDInstance) return;

    APawn* P = GetPawn();
    if (!P) return;

    if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P))
    {
        if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
        {
            using FSA = UFS_AttributeSet_Stats; // your set

            HUDInstance->InitializeForASC(
                ASC,
                FSA::GetHealthAttribute(),
                FSA::GetMaxHealthAttribute(),
                FSA::GetStaminaAttribute(),
                FSA::GetMaxStaminaAttribute()
            );

            HUDInstance->ActivateWidget(); // makes sure delegates are bound
        }
    }
}