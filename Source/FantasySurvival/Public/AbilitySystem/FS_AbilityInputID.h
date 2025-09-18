// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FS_AbilityInputID.generated.h"

UENUM(BlueprintType)
enum class EFSAbilityInputID : uint8
{
	None			UMETA(DisplayName = "None"),
	PrimaryAttack	UMETA(DisplayName = "PrimaryAttack"),
	Ability1		UMETA(DisplayName = "Ability1"),
	// Add more later (Ability2, Dodge, Ult, etc.)
};