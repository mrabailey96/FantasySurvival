// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FS_PlayerClass.generated.h"

UENUM(BlueprintType)
enum class EFSPlayerClass : uint8
{
	Warrior UMETA(DisplayName = "Warrior"),
	Mage    UMETA(DisplayName = "Mage"),
	Ranger  UMETA(DisplayName = "Ranger"),
	Assassin UMETA(DisplayName = "Assassin")
};