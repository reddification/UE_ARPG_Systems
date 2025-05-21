

#include "BehaviorTree/Tasks/BTTask_LookAt.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "NativeGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"


UBTTask_LookAt::UBTTask_LookAt()
{
	NodeName = "Look at";
	LookAtBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAt, LookAtBBKey), AActor::StaticClass());
	LookAtBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAt, LookAtBBKey));
	LookAtBBKey.AddRotatorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAt, LookAtBBKey));
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_LookAt::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	const FVector PawnLocation = Pawn->GetActorLocation();
	FRotator DesiredRotation = FRotator::ZeroRotator;
	if (LookAtBBKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		const FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(LookAtBBKey.SelectedKeyName);
		DesiredRotation = (bInverse ? (PawnLocation - TargetLocation) : (TargetLocation - PawnLocation)).ToOrientationRotator();
	}
	else if (LookAtBBKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		const AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(LookAtBBKey.SelectedKeyName));
		if(!IsValid(TargetActor))
		{
			return EBTNodeResult::Failed;
		}
		
		DesiredRotation = (bInverse ? (PawnLocation - TargetActor->GetActorLocation()) : (TargetActor->GetActorLocation() - PawnLocation)).ToOrientationRotator();
	}
	else if (LookAtBBKey.SelectedKeyType == UBlackboardKeyType_Rotator::StaticClass())
	{
		DesiredRotation = OwnerComp.GetBlackboardComponent()->GetValueAsRotator(LookAtBBKey.SelectedKeyName);
		if (bInverse)
			DesiredRotation.Yaw += 180.f;
	}

	if (DesiredRotation.Equals(Pawn->GetActorRotation(), 2.5f))
		return EBTNodeResult::Succeeded;
	
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return EBTNodeResult::Failed;

	Super::ExecuteTask(OwnerComp, NodeMemory);
	Npc->LookAt(DesiredRotation);
	return EBTNodeResult::InProgress;
}

FString UBTTask_LookAt::GetStaticDescription() const
{
	return FString::Printf(TEXT("Look at %s, %s\n%s"), *LookAtBBKey.SelectedKeyName.ToString(), bInverse ? TEXT("Inverse") : TEXT("Directly"), *Super::GetStaticDescription());
}

void UBTTask_LookAt::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	const UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		LookAtBBKey.ResolveSelectedKey(*BBAsset);
	}

	CompletedMessageTag = AIGameplayTags::AI_BrainMessage_LookAt_Completed;
}

void UBTTask_LookAt::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
	if (auto Npc = Cast<INpc>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		Npc->CancelLookAt();
	}
}
