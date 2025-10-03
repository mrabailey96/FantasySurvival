// Fill out your copyright notice in the Description page of Project Settings.


#include "System/FS_GameMode.h"
#include "Characters/FS_PlayerCharacter.h"
#include "AbilitySystem/FS_AS_PlayerState.h"

AFS_GameMode::AFS_GameMode()
{
	// Assign defaults (Swap to our custom PC/HUD when ready)
	DefaultPawnClass = AFS_PlayerCharacter::StaticClass();
	PlayerStateClass = AFS_AS_PlayerState::StaticClass();
}
