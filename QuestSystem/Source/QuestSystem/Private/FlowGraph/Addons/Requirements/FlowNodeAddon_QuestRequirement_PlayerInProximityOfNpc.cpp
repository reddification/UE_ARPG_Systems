// 


#include "FlowGraph/Addons/Requirements/FlowNodeAddon_QuestRequirement_PlayerInProximityOfNpc.h"

#include "Data/QuestTypes.h"
#include "Interfaces/QuestCharacter.h"
#include "Subsystems/QuestNpcSubsystem.h"

bool UFlowNodeAddon_QuestRequirement_PlayerInProximityOfNpc::EvaluatePredicate_Implementation() const
{
	if (!Super::EvaluatePredicate_Implementation())
		return false;

	auto QuestSystemContext = GetQuestSystemContext();
	const FVector PlayerEyesLocation = QuestSystemContext.Player->GetQuestCharacterEyesLocation();
	auto Npc = QuestSystemContext.NpcSubsystem->FindNpc(NpcId, PlayerEyesLocation);
	if (Npc == nullptr)
		return false;

	if (bTraceVisibility)
	{
		FHitResult HitResult;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(Cast<AActor>(QuestSystemContext.Player.GetObject()));
		auto NpcActor = Cast<AActor>(Npc.GetObject());
		const FVector TraceEnd = PlayerEyesLocation + (Npc->GetQuestNpcEyesLocation() - PlayerEyesLocation).GetSafeNormal() * Proximity; 
		const bool bBlockingHit = QuestSystemContext.World->LineTraceSingleByChannel(HitResult, PlayerEyesLocation, TraceEnd,
			ECC_Visibility, CollisionQueryParams);
		return bBlockingHit && HitResult.GetActor() == NpcActor;
	}
	else
	{
		return (PlayerEyesLocation - Npc->GetQuestNpcEyesLocation()).SizeSquared() < Proximity * Proximity;
	}
}
