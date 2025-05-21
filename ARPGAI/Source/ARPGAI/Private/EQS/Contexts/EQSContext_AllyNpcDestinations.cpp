// 


#include "EQS/Contexts/EQSContext_AllyNpcDestinations.h"

#include "AIController.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Navigation/PathFollowingComponent.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"

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
	const UNpcActivitySquadSubsystem* NpcRegistrationSubsystem = UNpcActivitySquadSubsystem::Get(QuerierPawn);
	const TArray<APawn*> Allies = NpcRegistrationSubsystem->GetAllies(QuerierPawn, true);
	for (const auto AllyPawn : Allies)
	{
		auto AllyController = Cast<AAIController>(AllyPawn->GetController());
		if (AllyController->GetMoveStatus() == EPathFollowingStatus::Type::Idle)
			continue;

		auto Path = AllyController->GetPathFollowingComponent()->GetPath();
		if (!ensure(Path.IsValid() && Path->IsValid()))
			continue;

		AlliesDestinations.Add(Path->GetEndLocation());

#if WITH_EDITOR
		FBox Box = FBox::BuildAABB(Path->GetEndLocation(), FVector(30, 30, 90));
		UE_VLOG_BOX(QuerierPawn, LogARPGAI, VeryVerbose, Box, FColor::White, TEXT("Ally predicted location %s"), *AllyPawn->GetName());
#endif
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, AlliesDestinations);
}
