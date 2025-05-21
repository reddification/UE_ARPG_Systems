// Fill out your copyright notice in the Description page of Project Settings.
#include "BehaviorTree/Decorators/BTDecorator_ReleaseSmartObject.h"

#include "BlackboardKeyType_SOClaimHandle.h"
#include "SmartObjectSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_ReleaseSmartObject::UBTDecorator_ReleaseSmartObject()
{
	NodeName = "Release smart object";
	bNotifyDeactivation = true;
	ClaimedSmartObjectClaimHandleBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_SOClaimHandle>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_ReleaseSmartObject, ClaimedSmartObjectClaimHandleBBKey)));
	ActiveSmartObjectClaimHandleBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_SOClaimHandle>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_ReleaseSmartObject, ActiveSmartObjectClaimHandleBBKey)));
	InteractionActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_ReleaseSmartObject, InteractionActorBBKey), AActor::StaticClass());
}

void UBTDecorator_ReleaseSmartObject::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	Blackboard->ClearValue(InteractionActorBBKey.SelectedKeyName);

	auto ClaimedSmartObjectClaimHandle = Blackboard->GetValue<UBlackboardKeyType_SOClaimHandle>(ClaimedSmartObjectClaimHandleBBKey.SelectedKeyName);
	auto ActiveSmartObjectClaimHandle = Blackboard->GetValue<UBlackboardKeyType_SOClaimHandle>(ActiveSmartObjectClaimHandleBBKey.SelectedKeyName);
	if (!ClaimedSmartObjectClaimHandle.IsValid() && !ActiveSmartObjectClaimHandle.IsValid())
		return;

	auto World = SearchData.OwnerComp.GetWorld();
	if (World == nullptr)
		return;
	
	if (USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>())
	{
		if (ClaimedSmartObjectClaimHandle.IsValid())
		{
			SmartObjectSubsystem->Release(ClaimedSmartObjectClaimHandle);
			Blackboard->ClearValue(ClaimedSmartObjectClaimHandleBBKey.SelectedKeyName);
		}
		
		if (ActiveSmartObjectClaimHandle.IsValid())
		{
			if (ActiveSmartObjectClaimHandle.SlotHandle != ClaimedSmartObjectClaimHandle.SlotHandle)
				SmartObjectSubsystem->Release(ActiveSmartObjectClaimHandle);
			
			Blackboard->ClearValue(ActiveSmartObjectClaimHandleBBKey.SelectedKeyName);
		}
	}
}

FString UBTDecorator_ReleaseSmartObject::GetStaticDescription() const
{
	return FString::Printf(TEXT("Release smart object on deactivation\nClaimed SO claim handle BB: %s\nActive SO claim handle BB: %s\nInteraction actor BB: %s"),
		*ClaimedSmartObjectClaimHandleBBKey.SelectedKeyName.ToString(),
		*ActiveSmartObjectClaimHandleBBKey.SelectedKeyName.ToString(),
		*InteractionActorBBKey.SelectedKeyName.ToString());
}