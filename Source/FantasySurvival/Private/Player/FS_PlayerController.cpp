// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/FS_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "UI/FS_PlayerHUDWidget.h"
#include "Player/FS_PlayerState.h"

AFS_PlayerController::AFS_PlayerController()
{
}

void AFS_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LP = GetLocalPlayer();

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (Subsystem && InputMappingContext)
    {
        Subsystem->AddMappingContext(InputMappingContext, /*Priority*/ 0);
    }

    if (PlayerHUDClass)
    {
        PlayerHUD = CreateWidget<UFS_PlayerHUDWidget>(this, PlayerHUDClass);
        if (PlayerHUD)
        {
            PlayerHUD->AddToViewport(0);
            if (AFS_PlayerState* PS = GetPlayerState<AFS_PlayerState>())
            {
                PlayerHUD->InitializeFromPlayerState(PS);
            }
        }
    }

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
}
