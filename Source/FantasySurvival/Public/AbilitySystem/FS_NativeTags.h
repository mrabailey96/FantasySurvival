// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// Central registry for all native gameplay tags used by the GAS

// States
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Blocking);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Attacking);

// Abilities
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_PrimaryAttack);

// Events (Montage Notifies)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_MeleeHitWindow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_ArcaneBolt_Spawn);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Cleave_HitWindow);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_ShadowStep_Teleport);

// Cooldowns
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_PrimaryAttack);