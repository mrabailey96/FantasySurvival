// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "FS_PlayerController.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UUserWidget;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AFS_PlayerController();


	// ---- Enhanced Input ----
	// Mapping Context
	UPROPERTY(EditDefaultsOnly, Category = "EnhancedInput|MappingContext")
	TObjectPtr<UInputMappingContext> InputMappingContext; // Assign in BP

	// ---- Player HUD ----
	UPROPERTY(EditDefaultsOnly, Category = "UI|PlayerHUD")
	TSubclassOf<UUserWidget> PlayerHUDClass;

	UPROPERTY(VisibleInstanceOnly, Category = "UI|PlayerHUD")
	TObjectPtr<UUserWidget> PlayerHUD;

protected:
	virtual void BeginPlay() override;

};
