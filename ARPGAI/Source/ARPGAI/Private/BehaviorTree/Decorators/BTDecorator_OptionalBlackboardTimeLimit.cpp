#include "BehaviorTree/Decorators/BTDecorator_OptionalBlackboardTimeLimit.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_OptionalBlackboardTimeLimit::UBTDecorator_OptionalBlackboardTimeLimit()
{
	NodeName = "Optional Blackboard Time Limit";
	TimeLimitBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_OptionalBlackboardTimeLimit, TimeLimitBBKey));
	IsIndefiniteBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_OptionalBlackboardTimeLimit, IsIndefiniteBBKey));
	IsIndefiniteBBKey.AllowNoneAsValue(true);
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	bTickIntervals = true;
	// time limit always abort current branch
	bAllowAbortLowerPri = false;
	bAllowAbortNone = false;
	FlowAbortMode = EBTFlowAbortMode::Self;
	
	bNotifyBecomeRelevant = true;
	bNotifyDeactivation = true;
	bNotifyProcessed = true;
}

void UBTDecorator_OptionalBlackboardTimeLimit::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

#if WITH_EDITOR
	bool bRunIndefinitely_Debug = OwnerComp.GetBlackboardComponent()->GetValueAsBool(IsIndefiniteBBKey.SelectedKeyName);
	if (!ensure (bRunIndefinitely_Debug == false))
	{
		// 26.11.2024 @AK:
		// this shit shouldn't even happen wtf TODO fix it mf
		return;
	}
#endif
	
	auto BTMemory = reinterpret_cast<FBTMemory_OptionalBlackboardTimeLimit*>(NodeMemory);
	BTMemory->bTimeLimitExpired = true;
	SetNextTickTime(NodeMemory, FLT_MAX);
	OwnerComp.RequestExecution(this);
}

void UBTDecorator_OptionalBlackboardTimeLimit::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	UBlackboardComponent* Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	if (Blackboard->GetValueAsBool(IsIndefiniteBBKey.SelectedKeyName))
		return;
	
	FBTAuxiliaryMemory* DecoratorMemory = GetNodeMemory<FBTAuxiliaryMemory>(SearchData);
	if (NodeResult == EBTNodeResult::Aborted && DecoratorMemory->NextTickRemainingTime > 0.f && bSaveRemainingGoalTime)
		Blackboard->SetValueAsFloat(TimeLimitBBKey.SelectedKeyName, DecoratorMemory->NextTickRemainingTime);

	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_OptionalBlackboardTimeLimit::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	bool bIndefinite = IsIndefiniteBBKey.IsNone() ? false : Blackboard->GetValueAsBool(IsIndefiniteBBKey.SelectedKeyName);
	if (!bIndefinite)
	{
		auto BTMemory = reinterpret_cast<FBTMemory_OptionalBlackboardTimeLimit*>(NodeMemory);
		BTMemory->bTimeLimitSet = true;
		BTMemory->bTimeLimitExpired = false;
	}
	
	SetNextTickTime(NodeMemory, bIndefinite ? FLT_MAX : Blackboard->GetValueAsFloat(TimeLimitBBKey.SelectedKeyName));
}

void UBTDecorator_OptionalBlackboardTimeLimit::OnNodeProcessed(FBehaviorTreeSearchData& SearchData,
                                                               EBTNodeResult::Type& NodeResult)
{
	Super::OnNodeProcessed(SearchData, NodeResult);
	auto NodeMemory = GetNodeMemory<FBTMemory_OptionalBlackboardTimeLimit>(SearchData);
	if (NodeResult != EBTNodeResult::Aborted && NodeMemory->bTimeLimitSet && NodeMemory->bTimeLimitExpired && bTreatTimeoutAsSuccess)
		NodeResult = EBTNodeResult::Succeeded;

	NodeMemory->bTimeLimitExpired = false;
	NodeMemory->bTimeLimitSet = false;
}

FString UBTDecorator_OptionalBlackboardTimeLimit::GetStaticDescription() const
{
	return FString::Printf(TEXT("Time limit BB key: %s\nIsIndefinite BB Key: %s%s%s"),
		*TimeLimitBBKey.SelectedKeyName.ToString(), *IsIndefiniteBBKey.SelectedKeyName.ToString(),
		bTreatTimeoutAsSuccess ? TEXT("\nTreat timeout as success") : TEXT(""),
		bSaveRemainingGoalTime ? TEXT("\nSave remaining NPC goal time") : TEXT(""));
}

void UBTDecorator_OptionalBlackboardTimeLimit::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto Blackboard = Asset.GetBlackboardAsset())
	{
		TimeLimitBBKey.ResolveSelectedKey(*Blackboard);
		IsIndefiniteBBKey.ResolveSelectedKey(*Blackboard);
	}
}
