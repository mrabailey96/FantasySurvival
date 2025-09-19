// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FS_PlayerController.generated.h"

class UFS_HUDWidget;
class UCommonActivatableWidget;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AFS_PlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// Assign a BP subclass of UFS_HUDWidget in defaults
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFS_HUDWidget> HUDWidgetClass;

	void ApplyGameInputMode();
	void ApplyMenuInputMode(UUserWidget* FocusWidget = nullptr);

private:
	UPROPERTY()
	TObjectPtr<UFS_HUDWidget> HUDInstance = nullptr;

	void EnsureHUD();
	void WireASC();

};
