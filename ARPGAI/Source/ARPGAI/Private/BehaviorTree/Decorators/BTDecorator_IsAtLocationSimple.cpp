


#include "BehaviorTree/Decorators/BTDecorator_IsAtLocationSimple.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTDecorator_IsAtLocationSimple::UBTDecorator_IsAtLocationSimple()
{
	NodeName = "Is at location simple";
	LocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsAtLocationSimple, LocationBBKey));
	LocationBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsAtLocationSimple, LocationBBKey), AActor::StaticClass());
}

bool UBTDecorator_IsAtLocationSimple::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	FVector TargetLocation = FVector::ZeroVector;
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	const FBlackboard::FKey MyID = MyBlackboard->GetKeyID(LocationBBKey.SelectedKeyName);
	const TSubclassOf<UBlackboardKeyType> TargetKeyType = MyBlackboard->GetKeyType(MyID);
	const FVector AILocation = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
	if (TargetKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		const AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(LocationBBKey.SelectedKeyName));
		//If match is over, GameMode will remove player from level, this can happen after BB entry decorator has ran, and it will not abort sub tree
		if(TargetActor == nullptr)
		{
			return false;
		}
		
		TargetLocation = TargetActor->GetActorLocation();
		if(bUseCollisionVolume)
		{
			const float CollisionRadius = TargetActor->GetSimpleCollisionRadius();
			return (FVector::Distance(TargetLocation, AILocation) - CollisionRadius) < AcceptableDistance;
		}
	}
	else if (TargetKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(LocationBBKey.SelectedKeyName);
	}
	return FVector::DistSquared(TargetLocation, AILocation) < (AcceptableDistance * AcceptableDistance);
}

FString UBTDecorator_IsAtLocationSimple::GetStaticDescription() const
{
	return FString::Printf(TEXT("Is at location %s with acceptable distance = %.2f"), *LocationBBKey.SelectedKeyName.ToString(),
		AcceptableDistance);
}
