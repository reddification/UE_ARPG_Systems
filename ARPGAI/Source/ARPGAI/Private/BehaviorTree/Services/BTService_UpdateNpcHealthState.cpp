#include "BehaviorTree/Services/BTService_UpdateNpcHealthState.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Interfaces/NpcAliveActor.h"

UBTService_UpdateNpcHealthState::UBTService_UpdateNpcHealthState()
{
	NodeName = "Update NPC Health State";
	
	OptionalCurrentTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateNpcHealthState, OptionalCurrentTargetBBKey), AActor::StaticClass());
	OutNormalizedHealthBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateNpcHealthState, OutNormalizedHealthBBKey));
	OutIndividualAccumulatedDamageBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateNpcHealthState, OutIndividualAccumulatedDamageBBKey));
	OutTotalAccumulatedDamageBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateNpcHealthState, OutTotalAccumulatedDamageBBKey));
	
	OptionalCurrentTargetBBKey.AllowNoneAsValue(true);
	OutNormalizedHealthBBKey.AllowNoneAsValue(true);
	OutIndividualAccumulatedDamageBBKey.AllowNoneAsValue(true);
	OutTotalAccumulatedDamageBBKey.AllowNoneAsValue(true);
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_UpdateNpcHealthState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UpdateValues(OwnerComp);
}

void UBTService_UpdateNpcHealthState::UpdateValues(UBehaviorTreeComponent& OwnerComp)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto AIController = OwnerComp.GetAIOwner();
	auto PerceptionComponent = Cast<UNpcPerceptionComponent>(AIController->GetAIPerceptionComponent());
	auto AliveCreatureInterface = Cast<INpcAliveActor>(AIController->GetPawn());
	const float MaxHealth = AliveCreatureInterface->GetMaxHealth_NPC();
	
	if (OutNormalizedHealthBBKey.IsSet())
		Blackboard->SetValueAsFloat(OutNormalizedHealthBBKey.SelectedKeyName, AliveCreatureInterface->GetHealth_NPC() / MaxHealth);	
	
	if (OutTotalAccumulatedDamageBBKey.IsSet())
		Blackboard->SetValueAsFloat(OutTotalAccumulatedDamageBBKey.SelectedKeyName, PerceptionComponent->GetAccumulatedDamage(false) / MaxHealth);
	
	if (OutIndividualAccumulatedDamageBBKey.IsSet() && OptionalCurrentTargetBBKey.IsSet())
		if (auto CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(OptionalCurrentTargetBBKey.SelectedKeyName)))
			if (const auto* CharacterPerceptionData = PerceptionComponent->GetShortTermCharactersMemory(CurrentTarget))
				Blackboard->SetValueAsFloat(OutIndividualAccumulatedDamageBBKey.SelectedKeyName, CharacterPerceptionData->ShortTermAccumulatedDamage / MaxHealth);
}

void UBTService_UpdateNpcHealthState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UpdateValues(OwnerComp);
}

void UBTService_UpdateNpcHealthState::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		if (OutNormalizedHealthBBKey.IsSet())
			Blackboard->ClearValue(OutNormalizedHealthBBKey.SelectedKeyName);
		
		if (OutTotalAccumulatedDamageBBKey.IsSet())
			Blackboard->ClearValue(OutTotalAccumulatedDamageBBKey.SelectedKeyName);
		
		if (OutIndividualAccumulatedDamageBBKey.IsSet() && OptionalCurrentTargetBBKey.IsSet())
			Blackboard->ClearValue(OutIndividualAccumulatedDamageBBKey.SelectedKeyName);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_UpdateNpcHealthState::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		OptionalCurrentTargetBBKey.ResolveSelectedKey(*BB);
		OutNormalizedHealthBBKey.ResolveSelectedKey(*BB);
		OutTotalAccumulatedDamageBBKey.ResolveSelectedKey(*BB);
		OutIndividualAccumulatedDamageBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTService_UpdateNpcHealthState::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Normalized Health BB: %s\n[out]Total accumulated damage BB: %s\n[optional]Current target BB:%s\n[out]Individual accumulated damage BB: %s\n%s"), 
		 *OutNormalizedHealthBBKey.SelectedKeyName.ToString(),  *OutTotalAccumulatedDamageBBKey.SelectedKeyName.ToString(),
		 *OptionalCurrentTargetBBKey.SelectedKeyName.ToString(), *OutIndividualAccumulatedDamageBBKey.SelectedKeyName.ToString(),
		 *Super::GetStaticDescription());
}
