// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Central registry for all native gameplay tags used by the Ability System.
// Include this header anywhere you need to reference these tags.

// States
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Blocking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Attacking);

// Abilities
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_PrimaryAttack);

// Events
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_MeleeHitWindow);

// Cooldowns
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_PrimaryAttack);