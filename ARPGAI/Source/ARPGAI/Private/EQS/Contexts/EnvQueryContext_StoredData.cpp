// Fill out your copyright notice in the Description page of Project Settings.

#include "EQS/Contexts/EnvQueryContext_StoredData.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

#include "Components/NpcComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_StoredData::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                    FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
		return;
	
	auto NpcComponent = QuerierPawn->FindComponentByClass<UNpcComponent>();
	if (NpcComponent == nullptr)
		return;
	
	if (NpcComponent)
	{
		if (AActor* Actor = NpcComponent->GetStoredActor(DataTag, bConsumeAfterReading))
		{
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Actor);
			UE_VLOG_CAPSULE(QuerierPawn, LogARPGAI, VeryVerbose, Actor->GetActorLocation() - 90.f, 90.f, 30, FQuat::Identity, FColor::White, TEXT("%s"), *DataTag.ToString());
		}
		else
		{
			FVector Location = NpcComponent->GetStoredLocation(DataTag, bConsumeAfterReading);
			if (Location != FAISystem::InvalidLocation)
			{
				UEnvQueryItemType_Point::SetContextHelper(ContextData, Location);
				UE_VLOG_LOCATION(QuerierPawn, LogARPGAI, VeryVerbose, Location + FVector::UpVector * 35.f, 30.f, FColor::White, TEXT("%s"), *DataTag.ToString());
			}
		}
	}
}
