#include "BehaviorTree/Tasks/Blackboard/BTTask_CopyLocation.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTTask_CopyLocation::UBTTask_CopyLocation()
{
	NodeName = "Copy location";
	SourceBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_CopyLocation, SourceBBKey));
	SourceBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_CopyLocation, SourceBBKey), AActor::StaticClass());
	OutputBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_CopyLocation, OutputBBKey));
}

EBTNodeResult::Type UBTTask_CopyLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	Blackboard->ClearValue(OutputBBKey.SelectedKeyName);
	bool bSet = false;
	if (SourceBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		if (auto Actor = Cast<AActor>(Blackboard->GetValueAsObject(SourceBBKey.SelectedKeyName)))
		{
			Blackboard->SetValueAsVector(OutputBBKey.SelectedKeyName, Actor->GetActorLocation());
			bSet = true;
		}
	}
	else if (ensure(SourceBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass()))
	{
		auto Location = Blackboard->GetValueAsVector(SourceBBKey.SelectedKeyName);
		if (Location != FVector::ZeroVector && Location != FAISystem::InvalidLocation)
		{
			Blackboard->SetValueAsVector(OutputBBKey.SelectedKeyName, Location);
			bSet = true;
		}
	}
	
	return bSet ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

void UBTTask_CopyLocation::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		SourceBBKey.ResolveSelectedKey(*BB);
		OutputBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTTask_CopyLocation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Copy location\n%s to %s"),
		*SourceBBKey.SelectedKeyName.ToString(), *OutputBBKey.SelectedKeyName.ToString());
}
