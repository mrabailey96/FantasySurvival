// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/Attributes/FS_AttributeSet_Stats.h"
#include "FS_AsyncTaskAttributeChanged.generated.h"

class UAbilitySystemComponent;

// Broadcasts when the watched attribute's "CurrentValue" changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFSAttributeChanged, float, NewValue);

/**
 * Async Blueprint node: Listen for changes to a single Gameplay Attribute on an ASC.
 * Usage:
 *   Task = ListenForAttributeChange(ASC, HealthAttr)
 *   Task->OnChanged.AddDynamic(this, &ThisClass::Handler)
 *   ...
 *   Task->EndTask()  // e.g., on widget destruct
 */
UCLASS(Blueprintable)
class FANTASYSURVIVAL_API UFS_AsyncTaskAttributeChanged : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	// Fires whenever the attribus current value changes on the ASC
	UPROPERTY(BlueprintAssignable)
	FOnFSAttributeChanged OnChanged;

	// Start Listening. Returns an async task objext we must hold on to (variable)
	// BlueprintInternalUseOnly hides it from regular call sites (as with other Async nodes)
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DisplayName = "Listen For Attribute Change", Category = "FS|GAS"))
	static UFS_AsyncTaskAttributeChanged* ListenForAttributeChange(UAbilitySystemComponent* ASC, FGameplayAttribute Attribute);

	// Stop Listening and free the object. Always call this (e.g. on Widget/Event Destruct)
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASCRef;

	FGameplayAttribute WatchedAttribute;
	FDelegateHandle DelegateHandle;

	void HandleAttributeChange(const FOnAttributeChangeData& Data);

	// UBlueprintAsyncActionBase
	virtual void Activate() override {} // Not used; We set up in factory function
};
