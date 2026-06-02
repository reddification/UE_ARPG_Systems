#include "BehaviorTree/Services/BTService_EvaluateTarget.h"

#include "AIController.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"

UBTService_EvaluateTarget::UBTService_EvaluateTarget()
{
	NodeName = "Evaluate Target";
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateTarget, TargetBBKey), AActor::StaticClass());
	OutAttitudeBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateTarget, OutAttitudeBBKey)));
	OutTargetTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateTarget, OutTargetTagsBBKey)));
	// OutAttackRangeBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_EvaluateTarget, OutAttackRangeBBKey));
	// OutNormalizedHealthBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_EvaluateTarget, OutNormalizedHealthBBKey));
	OutStaticThreatScoreBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_EvaluateTarget, OutStaticThreatScoreBBKey));
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_EvaluateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UpdatePropertiesInBlackboard(OwnerComp);
}

void UBTService_EvaluateTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		TargetBBKey.ResolveSelectedKey(*BB);
		OutAttitudeBBKey.ResolveSelectedKey(*BB);
		OutStaticThreatScoreBBKey.ResolveSelectedKey(*BB);
		OutTargetTagsBBKey.ResolveSelectedKey(*BB);
		// OutAttackRangeBBKey.ResolveSelectedKey(*BB);
		// OutNormalizedHealthBBKey.ResolveSelectedKey(*BB);
	}
}

void UBTService_EvaluateTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	FOnBlackboardChangeNotification BlackboardChangedNotification = FOnBlackboardChangeNotification::CreateUObject(this,
		&UBTService_EvaluateTarget::OnTargetChanged);
	Blackboard->RegisterObserver(TargetBBKey.GetSelectedKeyID(), this, BlackboardChangedNotification);
	UpdatePropertiesInBlackboard(OwnerComp);
}

void UBTService_EvaluateTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->UnregisterObserversFrom(this);
		ClearBlackboard(Blackboard);
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

EBlackboardNotificationResult UBTService_EvaluateTarget::OnTargetChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	if (BTComponent == nullptr)
		return EBlackboardNotificationResult::RemoveObserver;
	
	UpdatePropertiesInBlackboard(*BTComponent);	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTService_EvaluateTarget::UpdatePropertiesInBlackboard(UBehaviorTreeComponent& BTComponent)
{
	UBlackboardComponent* Blackboard = BTComponent.GetBlackboardComponent();
	if (auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName)))
	{
		auto NpcPerceptionComponent = Cast<UNpcPerceptionComponent>(BTComponent.GetAIOwner()->GetAIPerceptionComponent());
		if (auto CharacterPerceptionData = NpcPerceptionComponent->GetCharacterPerceptionData(Target))
		{
			Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(OutAttitudeBBKey.GetSelectedKeyID(), CharacterPerceptionData->Attitude.GetSingleTagContainer());
			Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(OutTargetTagsBBKey.GetSelectedKeyID(), CharacterPerceptionData->CharacterTags);
			Blackboard->SetValueAsFloat(OutStaticThreatScoreBBKey.SelectedKeyName, CharacterPerceptionData->StaticThreatScore);
			// Blackboard->SetValueAsFloat(OutAttackRangeBBKey.SelectedKeyName, CharacterPerceptionData->AttackRange);
			// Blackboard->SetValueAsFloat(OutNormalizedHealthBBKey.SelectedKeyName, CharacterPerceptionData->NormalizedHealth);
		}
		else
		{
			ClearBlackboard(Blackboard);
		}
	}
	else
	{
		ClearBlackboard(Blackboard);		
	}
}

void UBTService_EvaluateTarget::ClearBlackboard(UBlackboardComponent* Blackboard)
{
	Blackboard->ClearValue(OutStaticThreatScoreBBKey.GetSelectedKeyID());
	Blackboard->ClearValue(OutAttitudeBBKey.GetSelectedKeyID());
	Blackboard->ClearValue(OutTargetTagsBBKey.GetSelectedKeyID());
	// Blackboard->ClearValue(OutAttackRangeBBKey.GetSelectedKeyID());
	// Blackboard->ClearValue(OutNormalizedHealthBBKey.GetSelectedKeyID());
}

FString UBTService_EvaluateTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Evaluate target %s characteristics to:\nAttitude: %s\nStatic Threat Score: %s\nTags: %s\n%s"),
		*TargetBBKey.SelectedKeyName.ToString(), *OutAttitudeBBKey.SelectedKeyName.ToString(),
		*OutStaticThreatScoreBBKey.SelectedKeyName.ToString(), *OutTargetTagsBBKey.SelectedKeyName.ToString(),
		*Super::GetStaticDescription());
	
	// return FString::Printf(TEXT("Evaluate target %s characteristics to:\nAttitude: %s\nAttack Range: %s\nNormalized Health: %s\nThreat Score: %s"),
	// 	*TargetBBKey.SelectedKeyName.ToString(), *OutAttitudeBBKey.SelectedKeyName.ToString(), 
	// 	*OutAttackRangeBBKey.SelectedKeyName.ToString(), *OutNormalizedHealthBBKey.SelectedKeyName.ToString(),
	// 	*OutThreatScoreBBKey.SelectedKeyName.ToString());
}
