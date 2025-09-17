// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "FS_AbilityInputID.h"
#include "FS_AbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	// Convenience wrappers for Enhanced Input routing:
	void InputPressed(EFSAbilityInputID InputID) { AbilityLocalInputPressed((int32)InputID); }
	void InputReleased(EFSAbilityInputID InputID) { AbilityLocalInputReleased((int32)InputID); }
};
