// 


#include "BehaviorTree/Services/BehaviorEvaluators/BTService_BehaviorEvaluator_Base.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/Controller/NpcBehaviorEvaluatorComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/AiDataTypes.h"

UBTService_BehaviorEvaluator_Base::UBTService_BehaviorEvaluator_Base()
{
	NodeName = "Behavior Evaluator Base";
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	UtilityBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Base, UtilityBBKey));
	ActiveEvaluatorsTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTService_BehaviorEvaluator_Base, ActiveEvaluatorsTagsBBKey)));
}

void UBTService_BehaviorEvaluator_Base::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto AIController = OwnerComp.GetAIOwner();

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(NodeMemory);
	BTMemory->PerceptionComponent = AIController->FindComponentByClass<UNpcPerceptionComponent>();
	BTMemory->InactiveUtilityAccumulationRate = InactiveUtilityAccumulationRate.GetValue(Blackboard);
	BTMemory->InactiveUtilityRegressionOffset = InactiveUtilityRegressionOffset.GetValue(Blackboard);
	BTMemory->ActiveUtilityAccumulationRate = ActiveUtilityAccumulationRate.GetValue(Blackboard);
	BTMemory->ActiveUtilityRegressionOffset = ActiveUtilityRegressionOffset.GetValue(Blackboard);

	Blackboard->SetValueAsFloat(UtilityBBKey.SelectedKeyName, 0.f);
	FOnBlackboardChangeNotification ObserverDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_BehaviorEvaluator_Base::OnActiveBehaviorEvaluatorsChanged);
	Blackboard->RegisterObserver(ActiveEvaluatorsTagsBBKey.GetSelectedKeyID(), this, ObserverDelegate);
	auto CurrentlyActiveEvaluators = Blackboard->GetValue<UBlackboardKeyType_GameplayTag>(ActiveEvaluatorsTagsBBKey.SelectedKeyName);
	if (!CurrentlyActiveEvaluators.HasTag(BehaviorEvaluatorTag))
	{
		SetNextTickTime(NodeMemory, FLT_MAX);
		BTMemory->bBlocked = true;
	}

	FOnBlackboardChangeNotification UtilityChangeObserver = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_BehaviorEvaluator_Base::OnUtilityChangeParameterChanged);
	FBlackboard::FKey InactiveUARBBkey = InactiveUtilityAccumulationRate.GetKeyId(OwnerComp);
	if (InactiveUARBBkey != FBlackboard::InvalidKey)
		Blackboard->RegisterObserver(InactiveUARBBkey, this, UtilityChangeObserver);

	FBlackboard::FKey InactiveUDRBBkey = InactiveUtilityRegressionOffset.GetKeyId(OwnerComp);
	if (InactiveUDRBBkey != FBlackboard::InvalidKey)
		Blackboard->RegisterObserver(InactiveUDRBBkey, this, UtilityChangeObserver);

	FBlackboard::FKey ActiveUARBBkey = ActiveUtilityAccumulationRate.GetKeyId(OwnerComp);
	if (ActiveUARBBkey != FBlackboard::InvalidKey)
		Blackboard->RegisterObserver(ActiveUARBBkey, this, UtilityChangeObserver);

	FBlackboard::FKey ActiveUDRBBkey = ActiveUtilityRegressionOffset.GetKeyId(OwnerComp);
	if (ActiveUDRBBkey != FBlackboard::InvalidKey)
		Blackboard->RegisterObserver(ActiveUDRBBkey, this, UtilityChangeObserver);
	
	auto BEComponent = AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
	BEComponent->RegisterBehaviorEvaluator(this, BehaviorEvaluatorTag);
}

void UBTService_BehaviorEvaluator_Base::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto AIController = OwnerComp.GetAIOwner())
	{
		auto BEComponent = AIController->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
		BEComponent->UnregisterBehaviorEvaluator(BehaviorEvaluatorTag);
		
		auto BlackboardComponent = AIController->GetBlackboardComponent();
		BlackboardComponent->UnregisterObserversFrom(this);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_BehaviorEvaluator_Base::ChangeUtility(const float DeltaUtilityRaw, UBlackboardComponent* BlackboardComponent,
                                                      const float DeltaTime, const FBTMemory_BehaviorEvaluator_Base* BTMemory)
{
	float CurrentUtility = BlackboardComponent->GetValueAsFloat(UtilityBBKey.SelectedKeyName);
	float TrueDelta = DeltaUtilityRaw * DeltaTime * BTMemory->GetUtilityAccumulationRate();

	float NewUtility = FMath::Clamp(CurrentUtility + TrueDelta, 0.f, MaxUtility.GetValue(BlackboardComponent));
	BlackboardComponent->SetValueAsFloat(UtilityBBKey.SelectedKeyName, NewUtility);
}

EBlackboardNotificationResult UBTService_BehaviorEvaluator_Base::OnActiveBehaviorEvaluatorsChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	ensure(Key == ActiveEvaluatorsTagsBBKey.GetSelectedKeyID());
	auto CurrentlyActiveEvaluators = BlackboardComponent.GetValue<UBlackboardKeyType_GameplayTag>(ActiveEvaluatorsTagsBBKey.SelectedKeyName);
	auto BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	auto RawNodeMemory = BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this));
	FBTMemory_BehaviorEvaluator_Base* BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(RawNodeMemory);
	if (!BTMemory->bBlocked && !CurrentlyActiveEvaluators.HasTag(BehaviorEvaluatorTag))
	{
		SetNextTickTime(RawNodeMemory, FLT_MAX);
		BTMemory->bBlocked = true;
		BTComponent->GetBlackboardComponent()->SetValueAsFloat(UtilityBBKey.SelectedKeyName, 0.f);
	}
	else if (BTMemory->bBlocked && CurrentlyActiveEvaluators.HasTag(BehaviorEvaluatorTag))
	{
		SetNextTickTime(RawNodeMemory, 0.f);
		BTMemory->bBlocked = false;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

EBlackboardNotificationResult UBTService_BehaviorEvaluator_Base::OnUtilityChangeParameterChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	auto BehaviorTree = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BehaviorTree->GetNodeMemory(this, BehaviorTree->FindInstanceContainingNode(this)));
	if (Key == InactiveUtilityAccumulationRate.GetKeyId(*BehaviorTree))
		BTMemory->InactiveUtilityAccumulationRate = InactiveUtilityAccumulationRate.GetValue(BlackboardComponent);
	else if (Key == InactiveUtilityRegressionOffset.GetKeyId(*BehaviorTree))
		BTMemory->InactiveUtilityRegressionOffset = InactiveUtilityRegressionOffset.GetValue(BlackboardComponent);
	else if (Key == ActiveUtilityAccumulationRate.GetKeyId(*BehaviorTree))
		BTMemory->ActiveUtilityAccumulationRate = ActiveUtilityAccumulationRate.GetValue(BlackboardComponent);
	else if (Key == ActiveUtilityRegressionOffset.GetKeyId(*BehaviorTree))
		BTMemory->ActiveUtilityRegressionOffset = ActiveUtilityRegressionOffset.GetValue(BlackboardComponent);
	else
		return EBlackboardNotificationResult::RemoveObserver;
		
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_BehaviorEvaluator_Base::InitiateBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	auto BehaviorEvaluatorComponent = BTComponent->GetAIOwner()->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
	FGameplayTagContainer IndefinitelyBlockedEvaluators;
	for (const auto& EvaluatorBlockRequest : BlockEvaluatorsWhenActivated)
	{
		if (EvaluatorBlockRequest.bIndefinitely)
			IndefinitelyBlockedEvaluators.AddTagFast(EvaluatorBlockRequest.BehaviorEvaluatorTag);
		else
			BehaviorEvaluatorComponent->RequestEvaluatorBlocked(EvaluatorBlockRequest.BehaviorEvaluatorTag, EvaluatorBlockRequest.Duration);
	}

	if (!IndefinitelyBlockedEvaluators.IsEmpty())
		BehaviorEvaluatorComponent->RequestEvaluatorsBlocked(IndefinitelyBlockedEvaluators, true);

	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	BTMemory->bActive = true;
}

void UBTService_BehaviorEvaluator_Base::FinalizeBehaviorState(UBehaviorTreeComponent* BTComponent) const
{
	BTComponent->GetBlackboardComponent()->SetValueAsFloat(UtilityBBKey.SelectedKeyName, 0.f);
	auto BehaviorEvaluatorComponent = BTComponent->GetAIOwner()->FindComponentByClass<UNpcBehaviorEvaluatorComponent>();
	FGameplayTagContainer IndefinitelyBlockedEvaluators;
	for (const auto& EvaluatorBlockRequest : BlockEvaluatorsWhenActivated)
		if (EvaluatorBlockRequest.bIndefinitely)
			IndefinitelyBlockedEvaluators.AddTagFast(EvaluatorBlockRequest.BehaviorEvaluatorTag);

	if (!IndefinitelyBlockedEvaluators.IsEmpty())
		BehaviorEvaluatorComponent->RequestEvaluatorsBlocked(IndefinitelyBlockedEvaluators, false);

	auto BTMemory = reinterpret_cast<FBTMemory_BehaviorEvaluator_Base*>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	BTMemory->bActive = false;
}

void UBTService_BehaviorEvaluator_Base::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		ActiveEvaluatorsTagsBBKey.ResolveSelectedKey(*BB);
		UtilityBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTService_BehaviorEvaluator_Base::GetStaticDescription() const
{
	return FString::Printf(TEXT("Utility BB: %s\nAccumulate rate: %s\nBase: %s\nEvaluator id: %s\nActive evaluators BB: %s\n%s"),
		*UtilityBBKey.SelectedKeyName.ToString(), *InactiveUtilityAccumulationRate.ToString(), *InactiveUtilityRegressionOffset.ToString(),
		*BehaviorEvaluatorTag.ToString(), *ActiveEvaluatorsTagsBBKey.SelectedKeyName.ToString(), *Super::GetStaticDescription());
}
