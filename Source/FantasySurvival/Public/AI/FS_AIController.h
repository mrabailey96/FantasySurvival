// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FS_AIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API AFS_AIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AFS_AIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// BehaviorTree + Blackboard
	UPROPERTY(EditDefaultsOnly, Category = "FS|AI")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBlackboardComponent> BB = nullptr;

	// Blackboard keys (must match our BB asset)
	UPROPERTY(EditDefaultsOnly, Category = "FS|AI")
	FName Key_TargetActor = TEXT("SelfActor");

	UPROPERTY(EditDefaultsOnly, Category = "FS|AI")
	FName Key_AttackRange = TEXT("AttackRange");

	// Timer that re-acquires the player regularly
	FTimerHandle AcquireTimer;

	// Seconds between target refresh (low cost but responsive)
	UPROPERTY(EditDefaultsOnly, Category = "FS|AI")
	float AcquireInterval = 0.25f;

	// Set/Refresh TargetActor & AttackRange in Blackboard
	void AcquireTarget();

	// Helper to get best player to chase (SP: index 0, later: nearest)
	AActor* FindBestPlayer() const;
};
