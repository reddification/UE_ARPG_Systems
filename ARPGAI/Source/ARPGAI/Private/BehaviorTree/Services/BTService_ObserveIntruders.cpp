

#include "BehaviorTree/Services/BTService_ObserveIntruders.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISense_Sight.h"

UBTService_ObserveIntruders::UBTService_ObserveIntruders()
{
	NodeName = "Observe intruders";
	OutCurrentTargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_ObserveIntruders, OutCurrentTargetBBKey), AActor::StaticClass());
}

void UBTService_ObserveIntruders::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	APawn* CurrentIntruder = GetClosestIntruder(OwnerComp.GetAIOwner());
	if (CurrentIntruder)
	{
		UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
		if (BlackboardComponent->GetValueAsObject(OutCurrentTargetBBKey.SelectedKeyName) != CurrentIntruder)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(OutCurrentTargetBBKey.SelectedKeyName, CurrentIntruder);
		}
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(OutCurrentTargetBBKey.SelectedKeyName);
	}
}

APawn* UBTService_ObserveIntruders::GetClosestIntruder(AAIController* AIController) const
{
	UNpcComponent* MobComponent = AIController->GetPawn()->FindComponentByClass<UNpcComponent>();
	if (!ensure(MobComponent == nullptr))
	{
		return nullptr;
	}
	
	APawn* TargetCharacter = nullptr;
	float ClosestTargetDistance = FLT_MAX;
	const FAISenseID SenseID = UAISense::GetSenseID(UAISense_Sight::StaticClass());

	for (auto DataIt = AIController->GetPerceptionComponent()->GetPerceptualDataConstIterator(); DataIt; ++DataIt)
	{
		const bool bWasEverPerceived = DataIt->Value.HasKnownStimulusOfSense(SenseID);
		if (bWasEverPerceived)
		{
			if (DataIt->Value.Target.IsValid())
			{
				APawn* VisualTarget = Cast<APawn>(DataIt->Value.Target);
				if (!VisualTarget)
				{
					continue;
				}

				if (auto AliveCreature = Cast<INpcAliveCreature>(VisualTarget))
				{
					if (!AliveCreature->IsNpcActorAlive())
					{
						continue;
					}
				}

				FVector VisualTargetLocation = VisualTarget->GetActorLocation();
				FCollisionObjectQueryParams CollisionObjectQueryParams;
				ECollisionChannel GuardZoneCollisionChannel = ECC_Visibility; // TODO do something about it
				// UEngineTypes::ConvertToCollisionChannel(UEngineTypes::ConvertToObjectType(Gladius_ObjectChannel_GuardZone));
				CollisionObjectQueryParams.AddObjectTypesToQuery(GuardZoneCollisionChannel);
				FCollisionQueryParams CollisionQueryParams;
				TArray<FHitResult> HitResults;
				bool bVisualTargetTooCloseToGuardZone = false;
				// GetWorld()->LineTraceMultiByObjectType(HitResults, VisualTargetLocation,
				// GuardZoneLocation, CollisionObjectQueryParams);
				if (bVisualTargetTooCloseToGuardZone)
				{
					for (const FHitResult& HitResult : HitResults)
					{
						// if (HitResult.GetActor() == GuardZone)
						// {
						// 	if (HitResult.Distance < IntruderAlertRange)
						// 	{
						// 		if (HitResult.Distance < ClosestTargetDistance)
						// 		{
						// 			ClosestTargetDistance = HitResult.Distance;
						// 			TargetCharacter = VisualTarget;
						// 		}	
						// 	}
						//
						// 	break;
						// }
					}
				}
			}
		}
	}

	return TargetCharacter;
}

FString UBTService_ObserveIntruders::GetStaticDescription() const
{
	return FString::Printf(TEXT("Observe intruders %s at %.2f\n%s"), *OutCurrentTargetBBKey.SelectedKeyName.ToString(),
		IntruderAlertRange, *Super::GetStaticDescription());
}
