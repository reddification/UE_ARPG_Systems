// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_ActivateFormationWalking.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "Components/Controller/NpcSquadMemberComponent.h"

UBTDecorator_ActivateFormationWalking::UBTDecorator_ActivateFormationWalking()
{
	NodeName = TEXT("Activate formation walking");
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_ActivateFormationWalking::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (auto NpcComponent = SearchData.OwnerComp.GetAIOwner()->FindComponentByClass<UNpcSquadMemberComponent>())
		NpcComponent->SetFormationWalkingEnabled(true);
}

void UBTDecorator_ActivateFormationWalking::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto NpcComponent = SearchData.OwnerComp.GetAIOwner()->FindComponentByClass<UNpcSquadMemberComponent>())
		NpcComponent->SetFormationWalkingEnabled(false);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

FString UBTDecorator_ActivateFormationWalking::GetStaticDescription() const
{
	return TEXT("Prevents formation members from avoiding each other");
}
