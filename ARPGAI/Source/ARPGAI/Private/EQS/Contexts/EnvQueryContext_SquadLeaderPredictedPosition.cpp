// 
#include "EQS/Contexts/EnvQueryContext_SquadLeaderPredictedPosition.h"

#include "Data/LogChannels.h"
#include "Data/NpcSettings.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Subsystems/NpcSquadSubsystem.h"

void UEnvQueryContext_SquadLeaderPredictedPosition::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
	{
		return;
	}

	auto SquadLeader = QuerierPawn->GetWorld()->GetSubsystem<UNpcSquadSubsystem>()->GetSquadLeader(QuerierPawn);
	if (!SquadLeader)
		return;
	
	const float PredictionTime = GetDefault<UNpcSettings>()->FollowLeaderPredictionTime;
	FVector ExpectedLocation = SquadLeader->GetActorLocation() + SquadLeader->GetVelocity() * PredictionTime;
	UEnvQueryItemType_Point::SetContextHelper(ContextData, ExpectedLocation);

	UE_VLOG_CAPSULE(QuerierPawn, LogARPGAI, VeryVerbose, ExpectedLocation - FVector::UpVector * 90.f, 90.f, 30.f,
		FQuat::Identity, FColor::White, TEXT("Squad leader predicted location"));
}
