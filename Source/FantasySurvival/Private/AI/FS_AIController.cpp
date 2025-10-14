// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/FS_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/FS_EnemyCharacter.h"

AFS_AIController::AFS_AIController()
{
	bAttachToPawn = true;
}

void AFS_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (DefaultBehaviorTree)
	{
		RunBehaviorTree(DefaultBehaviorTree);
		BB = GetBlackboardComponent();
	}

	// Seed AttackRange from pawn
	if (BB)
	{
		if (const AFS_EnemyCharacter* Enemy = Cast<AFS_EnemyCharacter>(InPawn))
		{
			BB->SetValueAsFloat(Key_AttackRange, Enemy->AttackRange);
		}
	}

	// Start periodic acquisition (always chase player)
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(AcquireTimer, this, &AFS_AIController::AcquireTarget, AcquireInterval, true, 0.0f);
	}
}

void AFS_AIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AcquireTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void AFS_AIController::AcquireTarget()
{
	if (!BB) { BB = GetBlackboardComponent(); }
	if (!BB) return;

	if (AActor* Best = FindBestPlayer())
	{
		BB->SetValueAsObject(Key_TargetActor, Best);
		SetFocus(Best); // Optional: Smoother facing
	}
	// Never clear target; always chase even if far away
}

AActor* AFS_AIController::FindBestPlayer() const
{
	// Single-player: Just return player 0's pawn if valid
	if (APawn* P0 = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		return P0;
	}

	// Future (Multiplayer) We can pick nearest player like this:
	/*
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);

	AActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	const FVector MyLoc = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;

	for(AAtcor* A : Players)
	{
		// TODO: Filter to "Player" pawns only if we have NPCs using ACharacter
		const float D2 = FVector::DistSquared(MyLoc, A->GetActorLocation());
		if(D2 < BestDistSq)
		{
			Best = A;
			BestDistSq = D2;
		}
	}
	return Best;
	*/

	return nullptr;
}
