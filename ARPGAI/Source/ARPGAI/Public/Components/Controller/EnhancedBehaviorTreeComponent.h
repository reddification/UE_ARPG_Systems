// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Composites/BTComposite_Utility.h"
#include "BehaviorTree/Services/BTservice_ExclusiveStackedService.h"
#include "Data/NpcBehaviorsConfiguration.h"
#include "EnhancedBehaviorTreeComponent.generated.h"

struct FNpcDTR;

UCLASS()
class ARPGAI_API UEnhancedBehaviorTreeComponent : public UBehaviorTreeComponent
{
	GENERATED_BODY()
	
	struct FBTUtilityCompositeData
	{
		FBTUtilityCompositeData(const UBTComposite_Utility* Utility, uint8 ChildIndex, bool bSupressRest) 
			: Utility(Utility), ChildIndex(ChildIndex), bSupressRest(bSupressRest)
		{
		}

		TWeakObjectPtr<const UBTComposite_Utility> Utility;
		uint8 ChildIndex = 0;
		bool bSupressRest = false;
	};
	
	struct FBTUtilityCompositesContainer
	{
		TArray<FBTUtilityCompositeData> Items;
		FDelegateHandle ObserverHandle;
	};
	
public:
	virtual void PauseLogic(const FString& Reason) override;
	virtual EAILogicResuming::Type ResumeLogic(const FString& Reason) override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void HandleMessage(const FAIMessage& Message) override;
	void HandleMessageImmediately(const FAIMessage& Message);
	void LoadDynamicTrees(const FGameplayTagContainer& BehaviorTags, UBTCompositeNode* StartingNode);

	void InitializeNpc(const FNpcDTR* NpcDTR);
	
	void AddStackedService(const FName& Key, UBTservice_ExclusiveStackedService* Service);
	void RemoveStackedService(const FName& Key, UBTservice_ExclusiveStackedService* Service);

	void AddUtilityObserver(const FBlackboardKeySelector& BBKey, const UBTComposite_Utility* UtilityComposite, int ChildIndex, bool bSuppressesRest);
	void RemoveUtilityObserver(const FBlackboardKeySelector& BBKey, const UBTComposite_Utility* UtilityComposite);

protected:
	TArray<FBlackboardKeySelector> FlowControlBlackboardKeys;
	
	UPROPERTY()
	TObjectPtr<UNpcBehaviorsConfiguration> BehaviorsConfiguration;

private:
	void ResetFlowControlBlackboardKeys();
	EBlackboardNotificationResult OnUtilityChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key);
	
	bool bPausePending = false;
	TMap<FName, TArray<TWeakObjectPtr<UBTservice_ExclusiveStackedService>>> StackedServices;
	TMap<FBlackboard::FKey, FBTUtilityCompositesContainer> UtilityBlackboardObservers;
};
