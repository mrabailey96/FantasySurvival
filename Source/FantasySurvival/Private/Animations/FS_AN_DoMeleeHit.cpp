// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/FS_AN_DoMeleeHit.h"
#include "Combat/FS_EnemyCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UFS_AN_DoMeleeHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// Prefer the modern bool path; If it returns false, try the legacy 2-arg
	if (!Received_Notify(MeshComp, Animation, EventReference))
	{
		Received_Notify(MeshComp, Animation);
	}
}

bool UFS_AN_DoMeleeHit::Received_Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const
{
	if (!MeshComp) return false;

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UFS_EnemyCombatComponent* Combat = Owner->FindComponentByClass<UFS_EnemyCombatComponent>())
		{
			Combat->PerformMeleeWindow(); // apply damage at impact frame
			return true;
		}
	}

	return false;
}

bool UFS_AN_DoMeleeHit::Received_Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) const
{
	// Just forward to the 2-arg to keep logic in one place
	return Received_Notify(MeshComp, Animation);
}
