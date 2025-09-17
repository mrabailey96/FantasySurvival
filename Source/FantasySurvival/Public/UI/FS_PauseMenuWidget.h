// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FS_PauseMenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseMenuSimple);

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class FANTASYSURVIVAL_API UFS_PauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Button signals
	UPROPERTY(BlueprintAssignable, Category = "PauseMenu")
	FOnPauseMenuSimple OnResumeRequested;

	UPROPERTY(BlueprintAssignable, Category = "PauseMenu")
	FOnPauseMenuSimple OnSettingsRequested;

	UPROPERTY(BlueprintAssignable, Category = "PauseMenu")
	FOnPauseMenuSimple OnQuitToMainRequested;

	UPROPERTY(BlueprintAssignable, Category = "PauseMenu")
	FOnPauseMenuSimple OnQuitToDesktopRequested;

	// Call these from the widget’s Button OnClicked events
	UFUNCTION(BlueprintCallable, Category = "PauseMenu") void RequestResume() { OnResumeRequested.Broadcast(); }
	UFUNCTION(BlueprintCallable, Category = "PauseMenu") void RequestSettings() { OnSettingsRequested.Broadcast(); }
	UFUNCTION(BlueprintCallable, Category = "PauseMenu") void RequestQuitToMain() { OnQuitToMainRequested.Broadcast(); }
	UFUNCTION(BlueprintCallable, Category = "PauseMenu") void RequestQuitToDesktop() { OnQuitToDesktopRequested.Broadcast(); }
};
