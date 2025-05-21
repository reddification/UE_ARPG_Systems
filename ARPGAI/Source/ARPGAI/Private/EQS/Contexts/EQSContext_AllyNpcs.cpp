#include "EQS/Contexts/EQSContext_AllyNpcs.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"

void UEQSContext_AllyNpcs::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
	{
		return;
	}
	
	TArray<AActor*> ResultingActors;
	auto NpcSquadSubsystem = UNpcActivitySquadSubsystem::Get(QuerierPawn);
	const TArray<APawn*> NPCs = NpcSquadSubsystem->GetAllies(QuerierPawn, true);
	
#if WITH_EDITOR
	for (const auto AllyPawn : NPCs)
	{
		FBox Box = FBox::BuildAABB(AllyPawn->GetActorLocation(), FVector(30, 30, 90));
		UE_VLOG_BOX(QuerierPawn, LogARPGAI, VeryVerbose, Box, FColor::Emerald, TEXT("Ally %s"), *AllyPawn->GetName());
	}
#endif
	Algo::Transform(NPCs, ResultingActors, [](APawn* NPC){ return NPC; });
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, ResultingActors);
}
