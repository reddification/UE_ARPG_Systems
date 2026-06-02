#include "EQS/Contexts/EQSContext_AllyNpcs.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Subsystems/NpcSquadSubsystem.h"

void UEQSContext_AllyNpcs::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
	{
		return;
	}
	
	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(QuerierPawn);
	const TArray<APawn*> NPCs = NpcSquadSubsystem->GetAllies(QuerierPawn, CharacterQueryMode == ECharacterQueryMode::AliveOnly);
	TArray<AActor*> ResultingActors;
	ResultingActors.Reserve(NPCs.Num());	

	for (const auto AllyPawn : NPCs)
	{
#if WITH_EDITOR
		FBox Box = FBox::BuildAABB(AllyPawn->GetActorLocation(), FVector(30, 30, 90));
		UE_VLOG_BOX(QuerierPawn, LogARPGAI, VeryVerbose, Box, FColor::Emerald, TEXT("Ally %s"), *AllyPawn->GetName());
#endif
		
		bool bAdd = true;
		if (CharacterQueryMode == ECharacterQueryMode::DeadOnly)
		{
			bAdd = false;
			if (auto AliveInterface = Cast<INpcAliveCreature>(AllyPawn))
				bAdd = !AliveInterface->IsAlive_NpcAliveCreature();
		}
		
		if (bAdd)
			ResultingActors.Add(AllyPawn);
	}
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, ResultingActors);
}
