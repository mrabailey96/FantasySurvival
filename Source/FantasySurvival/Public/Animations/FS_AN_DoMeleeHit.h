// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FS_AN_DoMeleeHit.generated.h"

/**
 * 
 */
UCLASS()
class FANTASYSURVIVAL_API UFS_AN_DoMeleeHit : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// The engine calls this, which may forward to Received_Notify internally
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// Compatibility fallbacks (no 'override' so they compile on any branch)
	// Some branches call this legacy 2-arg version from Notify()
	virtual bool Received_Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const;

	// Modern 3-arg version many branches prefer
	virtual bool Received_Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) const;
};
