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
    /** Selected class from the main menu */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    EFSPlayerClass PendingClass = EFSPlayerClass::Warrior;

    /** Map to load from the main menu */
    UPROPERTY(BlueprintReadWrite, Category = "Menu")
    FName PendingMap = NAME_None;

    /** Setters (call from menu BP) */
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SetPendingClass(EFSPlayerClass InClass) { PendingClass = InClass; }

    UFUNCTION(BlueprintCallable, Category = "Menu")
    void SetPendingMap(FName InMap) { PendingMap = InMap; }

    /** Getters (used by Character/GameMode on server) */
    UFUNCTION(BlueprintPure, Category = "Menu")
    EFSPlayerClass GetPendingClass() const { return PendingClass; }

    UFUNCTION(BlueprintPure, Category = "Menu")
    FName GetPendingMap() const { return PendingMap; }

    /** Clear after returning to main menu */
    UFUNCTION(BlueprintCallable, Category = "Menu")
    void ClearPending()
    {
        PendingMap = NAME_None;
        // Optional: reset class if we want a default every time:
        // PendingClass = EFSPlayerClass::Warrior;
    }
};
