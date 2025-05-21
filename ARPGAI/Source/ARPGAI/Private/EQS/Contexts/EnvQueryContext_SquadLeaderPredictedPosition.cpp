// 


#include "EQS/Contexts/EnvQueryContext_SquadLeaderPredictedPosition.h"

#include "Components/Controller/NpcActivityComponent.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Subsystems/NpcActivitySquadSubsystem.h"

void UEnvQueryContext_SquadLeaderPredictedPosition::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
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
	const float PredictionTime = GetDefault<UNpcSettings>()->FollowLeaderPredictionTime;
	FVector ExpectedLocation = SquadLeaderPawn->GetActorLocation() + SquadLeaderPawn->GetVelocity() * PredictionTime;
	UEnvQueryItemType_Point::SetContextHelper(ContextData, ExpectedLocation);

	UE_VLOG_CAPSULE(QuerierPawn, LogARPGAI, VeryVerbose, ExpectedLocation - FVector::UpVector * 90.f, 90.f, 30.f,
		FQuat::Identity, FColor::White, TEXT("Squad leader predicted location"));
}
