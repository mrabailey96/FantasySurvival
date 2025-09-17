// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Characters/FS_PlayerClass.h"
#include "FS_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	// Set from the menu
	UPROPERTY(BlueprintReadWrite, Category = "Menu")
	EFSPlayerClass PendingClass = EFSPlayerClass::Warrior;

	UPROPERTY(BlueprintReadWrite, Category = "Menu")
	FName PendingMap = NAME_None;

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SetPendingClass(EFSPlayerClass InClass) { PendingClass = InClass; }

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SetPendingMap(FName InMap) { PendingMap = InMap; }

	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ClearPending() { PendingMap = NAME_None; }
};
