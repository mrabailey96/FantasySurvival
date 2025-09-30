// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "InputActionValue.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "FS_Character.generated.h"

class AFS_PlayerState;
class UFS_AbilitySystemComponent;
class UFS_PauseMenuWidget;

class UInputMappingContext;
class UInputAction;

class USpringArmComponent;
class UCameraComponent;

struct FFS_WeaponSpec;

UCLASS()
class FANTASYSURVIVAL_API AFS_Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AFS_Character();

	// GAS interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Possession/replication (to hook ASC → Pawn)
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// Input setup (Enhanced Input)
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Enhanced Input: assign these in the BP child */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;      // Axis2D (X=Right, Y=Forward)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;      // Axis2D (X=Yaw,  Y=Pitch)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;      // Bool (Pressed/Released)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_PrimaryAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Block;  // Bool (Pressed/Released)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Ability1;  // Digital trigger for your first ability

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Pause;

	// Pause menu class + live instance
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFS_PauseMenuWidget> PauseMenuClass;

	UPROPERTY()
	TObjectPtr<UFS_PauseMenuWidget> PauseMenuInstance;

	/** Cached ASC (owned by PlayerState) */
	UPROPERTY()
	TObjectPtr<UFS_AbilitySystemComponent> ASC;

	void ApplyClassAppearance();
	void ApplyClassWeapons();

	// Reusable helper used by ApplyClassWeapons
	UMeshComponent* CreateAndAttachWeapon(const FFS_WeaponSpec& Spec, FName DebugName);

	// Keep references so we can cleanly destroy/replace
	UPROPERTY(Transient) TObjectPtr<UMeshComponent> RightWeaponComp;
	UPROPERTY(Transient) TObjectPtr<UMeshComponent> LeftWeaponComp;

	/** Adds the IMC to the local player subsystem */
	void AddInputContext(class APlayerController* PC);

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 600.f;

	// Movement tuning
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchSpeed = 200.f;

	void StartSprint();
	void StopSprint();

	void BeginCrouch();
	void EndCrouch();

	void PrimaryAttack_Pressed();
	void PrimaryAttack_Released();

	void Block_Pressed();
	void Block_Released();

	/** Ability input handlers */
	void Ability1_Pressed();
	void Ability1_Released();

	/** Movement/look handlers */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// Toggle pause
	void TogglePauseMenu();

	// Handlers the widget will call back into
	UFUNCTION() void HandlePauseResume();
	UFUNCTION() void HandlePauseQuitToMain();
	UFUNCTION() void HandlePauseQuitToDesktop();

	// Helpers
	void ShowPauseMenu();
	void HidePauseMenu();
};
