// Fill out your copyright notice in the Description page of Project Settings.


#include "System/FS_GameMode.h"
#include "Characters/FS_Character.h"
#include "AbilitySystem/FS_PlayerState.h"
//#include "System/FS_PlayerController.h"

AFS_GameMode::AFS_GameMode()
{
	// Assign defaults (Swap to our custom PC/HUD when ready)
	DefaultPawnClass = AFS_Character::StaticClass();
	PlayerStateClass = AFS_PlayerState::StaticClass();
	//PlayerControllerClass = AFS_PlayerController::StaticClass();
}
