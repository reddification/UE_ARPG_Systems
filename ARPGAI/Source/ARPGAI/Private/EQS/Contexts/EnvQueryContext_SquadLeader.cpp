// 
#include "EQS/Contexts/EnvQueryContext_SquadLeader.h"

#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Subsystems/NpcSquadSubsystem.h"

void UEnvQueryContext_SquadLeader::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                  FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
	{
		return;
	}

	auto SquadLeader = QuerierPawn->GetWorld()->GetSubsystem<UNpcSquadSubsystem>()->GetSquadLeader(QuerierPawn);
	if (SquadLeader == nullptr)
		return;
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, SquadLeader);

	UE_VLOG_CAPSULE(QuerierPawn, LogARPGAI, VeryVerbose, SquadLeader->GetActorLocation() - FVector::UpVector * 90.f, 90.f, 30.f,
		FQuat::Identity, FColor::Cyan, TEXT("Squad leader current location"));
}
