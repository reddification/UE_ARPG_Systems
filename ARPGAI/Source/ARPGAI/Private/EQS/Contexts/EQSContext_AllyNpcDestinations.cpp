// 


#include "EQS/Contexts/EQSContext_AllyNpcDestinations.h"

#include "AIController.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Navigation/PathFollowingComponent.h"
#include "Subsystems/NpcSquadSubsystem.h"

void UEQSContext_AllyNpcDestinations::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                     FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
	{
		return;
	}
	
	TArray<FVector> AlliesDestinations;
	const UNpcSquadSubsystem* NpcRegistrationSubsystem = UNpcSquadSubsystem::Get(QuerierPawn);
	const TArray<APawn*> Allies = NpcRegistrationSubsystem->GetAllies(QuerierPawn, true, true);
	for (const auto AllyPawn : Allies)
	{
		auto AllyController = Cast<AAIController>(AllyPawn->GetController());
		if (AllyController == nullptr)
			continue;
		
		FVector PredictedLocation = AllyPawn->GetActorLocation();
		bool bGotPathDestination = false;
		if (AllyController->GetMoveStatus() != EPathFollowingStatus::Type::Idle)
		{
			auto Path = AllyController->GetPathFollowingComponent()->GetPath();
			if (ensure(Path.IsValid() && Path->IsValid()))
			{
				PredictedLocation = Path->GetEndLocation();
				bGotPathDestination = true;
			}
		} 
		
		if (!bGotPathDestination)
			if (bIgnoreNotMovingActors)
				continue;

		AlliesDestinations.Add(PredictedLocation);
		
#if WITH_EDITOR
		FBox Box = FBox::BuildAABB(PredictedLocation, FVector(30, 30, 90));
		UE_VLOG_BOX(QuerierPawn, LogARPGAI, VeryVerbose, Box, FColor::White, TEXT("Ally predicted location %s"), *AllyPawn->GetName());
#endif
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, AlliesDestinations);
}
