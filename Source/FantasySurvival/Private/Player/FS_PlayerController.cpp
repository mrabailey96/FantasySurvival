// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/FS_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

AFS_PlayerController::AFS_PlayerController()
{
}

void AFS_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LP = GetLocalPlayer();

	// Getting the Enhanced Input Local Player Subsystem Node and Plugging it into the Add Mapping Context Node (target)
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	// Setting the Mapping Context in the Add Mapping Context Node
	Subsystem->AddMappingContext(InputMappingContext, /* Priority */0);

	if (LP)
	{
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No EnhancedInputLocalPlayerSubsystem on LocalPlayer!"));
	}

	PlayerHUD = CreateWidget<UUserWidget>(this, PlayerHUDClass);

	PlayerHUD->AddToViewport(0);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
}
