// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/FS_NativeTags.h"

// States
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Blocking, "State.Blocking");
UE_DEFINE_GAMEPLAY_TAG(TAG_State_Attacking, "State.Attacking");

// Abilities
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_PrimaryAttack, "Ability.PrimaryAttack");

// Events
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_MeleeHitWindow, "Event.MeleeHitWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_ArcaneBolt_Spawn, "Event.ArcaneBolt.Spawn");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_Cleave_HitWindow, "Event.Cleave.HitWindow");
UE_DEFINE_GAMEPLAY_TAG(TAG_Event_ShadowStep_Teleport, "Event.ShadowStep.Teleport");

// Cooldowns
UE_DEFINE_GAMEPLAY_TAG(TAG_Cooldown_PrimaryAttack, "Cooldown.PrimaryAttack");