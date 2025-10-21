// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/FS_EnemyAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UFS_EnemyAnimInstance::UFS_EnemyAnimInstance()
{

}

void UFS_EnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UFS_EnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (!OwnerCharacter)
		{
			Speed = 0.0f;
			Direction = 0.0f;
			bIsInAir = false;

			return;
		}
	}

	const FVector Vel = OwnerCharacter->GetVelocity();
	const FVector PlanarVel = FVector(Vel.X, Vel.Y, 0.0f);
	Speed = PlanarVel.Size();

	const FRotator ActorRot = OwnerCharacter->GetActorRotation();
	ActorRotation = ActorRot;

	// Calculates direction relative to actor rotation, returns [-180..180] (UE helper)
	Direction = UKismetAnimationLibrary::CalculateDirection(Vel, ActorRot);

	const UCharacterMovementComponent* Move = OwnerCharacter->GetCharacterMovement();
	bIsInAir = (Move ? Move->IsFalling() : false);
}

