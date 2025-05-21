// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS/Contexts/EnvQueryContext_QuerierDestination.h"

#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Navigation/PathFollowingComponent.h"

void UEnvQueryContext_QuerierDestination::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                         FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
		return;

	auto AIController = Cast<AAIController>(QuerierPawn->GetController());
	FVector Destination = QuerierPawn->GetActorLocation();
	if (IsValid(AIController) && AIController->GetMoveStatus() != EPathFollowingStatus::Type::Idle)
	{
		auto Path = AIController->GetPathFollowingComponent()->GetPath();
        if (Path.IsValid() && Path->IsValid())
			Destination = Path->GetEndLocation();	        
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, Destination);
}
