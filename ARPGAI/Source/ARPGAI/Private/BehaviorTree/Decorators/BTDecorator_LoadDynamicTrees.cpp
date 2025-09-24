// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_LoadDynamicTrees.h"

#include "Components/Controller/EnhancedBehaviorTreeComponent.h"

UBTDecorator_LoadDynamicTrees::UBTDecorator_LoadDynamicTrees()
{
	NodeName = "Load dynamic trees";
	bNotifyActivation = true;
}

void UBTDecorator_LoadDynamicTrees::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (DynamicBehaviorTags.IsEmpty())
		return;

	if (auto EnhancedBTComponent = Cast<UEnhancedBehaviorTreeComponent>(&SearchData.OwnerComp))
		EnhancedBTComponent->LoadDynamicTrees(DynamicBehaviorTags, GetParentNode());
}

FString UBTDecorator_LoadDynamicTrees::GetStaticDescription() const
{
	return DynamicBehaviorTags.ToStringSimple().Replace(TEXT(", "), TEXT("\n"));
}