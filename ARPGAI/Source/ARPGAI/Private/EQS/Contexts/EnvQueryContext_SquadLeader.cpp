// 


#include "EQS/Contexts/EnvQueryContext_SquadLeader.h"

#include "Components/Controller/NpcActivityComponent.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"

void UEnvQueryContext_SquadLeader::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                  FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
	{
		return;
	}

	auto NpcActivityComponent = QuerierPawn->Controller->FindComponentByClass<UNpcActivityComponent>();
	if (!NpcActivityComponent)
		return;

	const FGuid& NpcSquadId = NpcActivityComponent->GetSquadId(); 
	if (!NpcSquadId.IsValid())
		return;

	auto SquadLeader = NpcActivityComponent->GetWorld()->GetSubsystem<UNpcActivitySquadSubsystem>()->GetSquadLeader(NpcSquadId);
	auto SquadLeaderPawn = SquadLeader->GetNpcPawn();
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, SquadLeaderPawn);

	UE_VLOG_CAPSULE(QuerierPawn, LogARPGAI, VeryVerbose, SquadLeaderPawn->GetActorLocation() - FVector::UpVector * 90.f, 90.f, 30.f,
		FQuat::Identity, FColor::Cyan, TEXT("Squad leader current location"));
}
