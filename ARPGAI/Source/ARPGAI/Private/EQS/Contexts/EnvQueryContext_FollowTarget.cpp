// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS/Contexts/EnvQueryContext_FollowTarget.h"

#include "Components/NpcComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_FollowTarget::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                   FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
	{
		return;
	}

	auto NpcComponent = QuerierPawn->FindComponentByClass<UNpcComponent>();
	if (!NpcComponent)
		return;

	auto FollowTarget = NpcComponent->GetFollowTarget();
	if (FollowTarget == nullptr)
		return;

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, FollowTarget);
}
