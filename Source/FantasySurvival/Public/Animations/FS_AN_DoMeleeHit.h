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
	virtual bool Received_Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEvent& EventReference) const;
};
