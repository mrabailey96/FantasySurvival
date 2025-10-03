// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "FS_PlayerCharacter.generated.h"

// Camera
class USpringArmComponent;
class UCameraComponent;

// Gameplay Ability System
class UAbilitySystemComponent;
class UFS_AttributeSet_Stats_;
class UGameplayEffect;

// Enhanced Input
class UInputMappingContext;
class UInputAction;

/**
 * Base Character:
 * - Implements IAbilitySystemInterface to expose ASC to abilities/effects
 * - Initializes ASC ActorInfo when possessed
 * - Applies GE_InitializeAttributes once to set starting stats
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_PlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFS_PlayerCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* IAbilitySystemInterface */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/* Called when a Controller possesses this pawn (server) */
	virtual void PossessedBy(AController* NewController) override;

	/* Called on clients when PlayerState replicates */
	virtual void OnRep_PlayerState() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ------------------ Camera ------------------
	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	// ------------------ Gameplay Ability System ------------------
	/* Cached pointer to the ASC on the PlayerState (not owned here) */
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	/* Cached pointer to our core AttributeSet on the PlayerState */
	UPROPERTY(Transient)
	UFS_AttributeSet_Stats_* StatsSet;

	// GameplayEffect used to initialize attributes (created as a Blueprint under Content)
	// Assign this in the Character's BP (e.g., BP_FS_PlayerCharacter) to our GE_InitializeAttributes
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Init")
	TSubclassOf<UGameplayEffect> DefaultInitializeAttributesEffect;

	// Initialize ASC ActorInfo and apply init attributes (server or owning client)
	void InitializeAbilitySystem();

	// Apply our DefaultInitializeAttributesEffect exactly once per spawn
	void ApplyDefaultAttributes();

	// ------------------ Enhanced Input (Assign these in the BP) ------------------
	// Adds the IMC to the local player subsystem
	void AddInputContext(class APlayerController* PC);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC; // Input Mapping Context

	UPROPERTY(EditDefaultsonly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move; // Axis2D (X=Right, Y=Forward)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look; // Axis2D (X=Yaw, Y=Pitch)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction>IA_Jump; // Action (Pressed, Released)

	// ------------------ Enhanced Input Functions ------------------
	// Movement/Look Handlers
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};
