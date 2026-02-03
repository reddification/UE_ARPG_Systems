// 


#include "EQS/Contexts/EnvQueryContext_OtherCombatantLocations.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcCombatLogicComponent.h"
#include "Data/LogChannels.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Navigation/PathFollowingComponent.h"

void UEnvQueryContext_OtherCombatantLocations::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                              FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	
	if (!ensure(!Roles.IsEmpty()))
		return;
	
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
		return;

	auto AIController = Cast<AAIController>(QuerierPawn->GetController());
	if (AIController == nullptr)
		return;
	
	auto NpcCombatLogicComponent = GetNpcCombatLogicComponent(QuerierPawn);
	if (!ensure(NpcCombatLogicComponent))
		return;
	
	auto CombatTarget = NpcCombatLogicComponent->GetPrimaryTargetActor();
	if (CombatTarget == nullptr)
	{
		UE_VLOG(AIController, LogARPGAI_EQS, Warning, TEXT("UEnvQueryContext_OtherCombatantLocations::ProvideContext: No combat target"));
		return;
	}
	
	auto EnemiesCoordinatorComponent = CombatTarget->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (EnemiesCoordinatorComponent == nullptr)
	{
		UE_VLOG(AIController, LogARPGAI_EQS, Warning, TEXT("UEnvQueryContext_OtherCombatantLocations::ProvideContext: No enemies coordinator on target %s"), *CombatTarget->GetName());
		return;
	}
	
	TArray<FVector> PeersLocations;
	TArray<APawn*> Peers;
	
	for (const auto& Role : Roles)
		Peers.Append(EnemiesCoordinatorComponent->GetEnemies(Role));
	
	for (const auto Peer : Peers)
	{
		if (Peer == QuerierPawn)
			continue;
		
		FVector PeerLocation = Peer->GetActorLocation();
		if (bPredictLocations)
		{
			auto PeerController = Cast<AAIController>(Peer->GetController());
			if (PeerController != nullptr)
			{
				if (PeerController->GetMoveStatus() != EPathFollowingStatus::Type::Idle)
				{
					auto Path = PeerController->GetPathFollowingComponent()->GetPath();
					if (ensure(Path.IsValid() && Path->IsValid()))
						PeerLocation = Path->GetEndLocation();
				}
			}
		}

#if WITH_EDITOR
		FBox Box = FBox::BuildAABB(PeerLocation, FVector(30, 30, 90));
		UE_VLOG_BOX(QuerierPawn, LogARPGAI, VeryVerbose, Box, FColor::White, TEXT("Ally predicted location %s"), *Peer->GetName());
#endif
		PeersLocations.Add(PeerLocation);
		
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, PeersLocations);
}
