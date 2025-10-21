// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/FS_AN_DoMeleeHit.h"
#include "Combat/FS_EnemyCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

bool UFS_AN_DoMeleeHit::Received_Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEvent& EventReference) const
{
	if (!MeshComp) return false;
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (UFS_EnemyCombatComponent* Combat = Owner->FindComponentByClass<UFS_EnemyCombatComponent>())
		{
			Combat->PerformMeleeWindow(); // does the sweep + applies GE
			return true;
		}
	}
	return false;
}
