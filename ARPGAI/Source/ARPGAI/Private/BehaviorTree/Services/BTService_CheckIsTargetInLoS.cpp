// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_CheckIsTargetInLoS.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTService_CheckIsTargetInLoS::UBTService_CheckIsTargetInLoS()
{
	NodeName = "Keep target in LoS";
	OutHasLoSBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckIsTargetInLoS, OutHasLoSBBKey));
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CheckIsTargetInLoS, TargetBBKey), AActor::StaticClass());
}

void UBTService_CheckIsTargetInLoS::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (Target == nullptr)
	{
		Blackboard->SetValueAsBool(OutHasLoSBBKey.SelectedKeyName, false);
		return;
	}
	
	bool bPreviousResult = Blackboard->GetValueAsBool(OutHasLoSBBKey.SelectedKeyName);
	bool bNewResult = false;

	auto Npc = OwnerComp.GetAIOwner()->GetPawn();
	const FVector NpcLocation = Npc->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	if ((Npc->GetActorForwardVector() | (TargetLocation - NpcLocation).GetSafeNormal()) > OwnerToTargetDotProductThreshold)
	{
		FActorPerceptionBlueprintInfo TargetPerception;
		bool bHasTargetPerception = OwnerComp.GetAIOwner()->GetAIPerceptionComponent()->GetActorsPerception(Target, TargetPerception);
		if (bHasTargetPerception)
		{
			auto SightSenseId = UAISense::GetSenseID(UAISense_Sight::StaticClass());
			for (const auto& PerceptionData : TargetPerception.LastSensedStimuli)
			{
				if (PerceptionData.Type == SightSenseId && PerceptionData.IsActive())
				{
					bNewResult = true;
					break;
				}
			}
		
		}
	}

	if (bNewResult != bPreviousResult)
	{
		Blackboard->SetValueAsBool(OutHasLoSBBKey.SelectedKeyName, bNewResult);
	}
}

FString UBTService_CheckIsTargetInLoS::GetStaticDescription() const
{
	return FString::Printf(TEXT("Check that the target is still in LoS\nTarget BB: %s\n[out] Is in LoS BB: %s\nDot product threshold = %.2f\n%s"),
		*TargetBBKey.SelectedKeyName.ToString(), *OutHasLoSBBKey.SelectedKeyName.ToString(), OwnerToTargetDotProductThreshold, *Super::GetStaticDescription());
}
