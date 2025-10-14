// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Async/FS_AsyncTaskAttributeChanged.h"
#include "AbilitySystemComponent.h"

UFS_AsyncTaskAttributeChanged* UFS_AsyncTaskAttributeChanged::ListenForAttributeChange(UAbilitySystemComponent* ASC, FGameplayAttribute Attribute)
{
	if (!ASC || !Attribute.IsValid())
	{
		return nullptr;
	}

	UFS_AsyncTaskAttributeChanged* Task = NewObject<UFS_AsyncTaskAttributeChanged>();
	Task->ASCRef = ASC;
	Task->WatchedAttribute = Attribute;

	// Bind to the ASCs attribute value change delegate
	FOnGameplayAttributeValueChange& Delegate = ASC->GetGameplayAttributeValueChangeDelegate(Attribute);

	Task->DelegateHandle = Delegate.AddUObject(Task, &UFS_AsyncTaskAttributeChanged::HandleAttributeChange);
	return Task;
}

void UFS_AsyncTaskAttributeChanged::HandleAttributeChange(const FOnAttributeChangeData& Data)
{
	// Broadcast only the new current value = Blueprints can pull MAx in their own graph
	OnChanged.Broadcast(Data.NewValue);
}

void UFS_AsyncTaskAttributeChanged::EndTask()
{
	if (ASCRef && DelegateHandle.IsValid())
	{
		FOnGameplayAttributeValueChange& Delegate = ASCRef->GetGameplayAttributeValueChangeDelegate(WatchedAttribute);
		Delegate.Remove(DelegateHandle);
	}
	SetReadyToDestroy();
	MarkAsGarbage(); // Allow GC to collect this object after the frame
}