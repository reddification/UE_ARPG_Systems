#include "BehaviorTree/Decorators/BTDecorator_StoreTaggedBlackboardData.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Components/NpcComponent.h"

UBTDecorator_StoreTaggedBlackboardData::UBTDecorator_StoreTaggedBlackboardData()
{
	NodeName = "Store NPC location";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	DataBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_StoreTaggedBlackboardData, DataBBKey));
	DataBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_StoreTaggedBlackboardData, DataBBKey), AActor::StaticClass());
}

void UBTDecorator_StoreTaggedBlackboardData::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto Blackboard = Asset.GetBlackboardAsset())
	{
		DataBBKey.ResolveSelectedKey(*Blackboard);
	}
}

void UBTDecorator_StoreTaggedBlackboardData::StoreData(const UBlackboardComponent* Blackboard, UNpcComponent* NpcComponent, FBTMemory_StoreTaggedBlackboardData* BTMemory)
{
	if (DataBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		if (BTMemory != nullptr)
			BTMemory->PreviousLocation = NpcComponent->GetStoredLocation(DataTag);
		
		NpcComponent->StoreTaggedLocation(DataTag, Blackboard->GetValueAsVector(DataBBKey.SelectedKeyName));
	}
	else if (DataBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (BTMemory != nullptr)
			BTMemory->PreviousActor = NpcComponent->GetStoredActor(DataTag);
		
		auto Actor = Cast<AActor>(Blackboard->GetValueAsObject(DataBBKey.SelectedKeyName));
		NpcComponent->StoreTaggedActor(DataTag, Actor);
	}
}

void UBTDecorator_StoreTaggedBlackboardData::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (!DataBBKey.IsSet())
		return;

	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	
	auto NpcComponent = SearchData.OwnerComp.GetAIOwner()->GetPawn()->FindComponentByClass<UNpcComponent>();
	if (!ensure(NpcComponent != nullptr))
		return;
	
	auto BTMemory = GetNodeMemory<FBTMemory_StoreTaggedBlackboardData>(SearchData);
	StoreData(Blackboard, NpcComponent, BTMemory);
	
	auto ObserverHadler = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_StoreTaggedBlackboardData::OnDataChanged);
	BTMemory->BlackboardDelegateHandle = Blackboard->RegisterObserver(DataBBKey.GetSelectedKeyID(), this, ObserverHadler);
}

void UBTDecorator_StoreTaggedBlackboardData::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (!DataBBKey.IsSet())
		return;

	if (auto AIController = SearchData.OwnerComp.GetAIOwner())
	{
		if (auto Pawn = AIController->GetPawn())
		{
			if (auto NpcComponent = Pawn->FindComponentByClass<UNpcComponent>())
			{
				auto BTMemory = GetNodeMemory<FBTMemory_StoreTaggedBlackboardData>(SearchData);
				if (BTMemory->PreviousActor.IsValid())
					NpcComponent->StoreTaggedActor(DataTag, BTMemory->PreviousActor.Get());
				else if (BTMemory->PreviousLocation != FAISystem::InvalidLocation)
					NpcComponent->StoreTaggedLocation(DataTag, BTMemory->PreviousLocation);
					
				if (BTMemory->BlackboardDelegateHandle.IsValid())
				{
					SearchData.OwnerComp.GetBlackboardComponent()->UnregisterObserver(DataBBKey.GetSelectedKeyID(), BTMemory->BlackboardDelegateHandle);
					BTMemory->BlackboardDelegateHandle.Reset();
				}
			}
		}
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

EBlackboardNotificationResult UBTDecorator_StoreTaggedBlackboardData::OnDataChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (Key != DataBBKey.GetSelectedKeyID())
	{
		ensure(false);
		return EBlackboardNotificationResult::RemoveObserver;
	}

	if (auto AIController = Cast<AAIController>(BlackboardComponent.GetOwner()))
		if (auto Pawn = AIController->GetPawn())
			if (auto NpcComponent = Pawn->FindComponentByClass<UNpcComponent>())
				StoreData(&BlackboardComponent, NpcComponent);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTDecorator_StoreTaggedBlackboardData::GetStaticDescription() const
{
	return FString::Printf(TEXT("Store NPC location %s\nParameter tag: %s"), *DataBBKey.SelectedKeyName.ToString(), *DataTag.ToString());
}
