// 


#include "FlowGraph/Nodes/Actions/FlowNode_QuestAction_DestroyActors.h"

#include "EngineUtils.h"
#include "GameplayTagAssetInterface.h"
#include "Data/QuestTypes.h"
#include "Subsystems/QuestNpcSubsystem.h"

EQuestActionExecuteResult UFlowNode_QuestAction_DestroyActors::ExecuteInternal(const FQuestSystemContext& Context)
{
	auto Base = Super::ExecuteInternal(Context);
	for (TActorIterator<AActor> ActorIterator(Context.NpcSubsystem->GetWorld(), ActorClass); ActorIterator; ++ActorIterator)
	{
		bool bDestroy = true;
		if (!GameplayTagFilter.IsEmpty())
		{
			if (auto GameplayTagActor = Cast<IGameplayTagAssetInterface>(*ActorIterator))
			{
				FGameplayTagContainer GameplayTagContainer;
				GameplayTagActor->GetOwnedGameplayTags(GameplayTagContainer);
				bDestroy = GameplayTagFilter.Matches(GameplayTagContainer);
			}
			else
				bDestroy = false;
		}

		if (bDestroy)
		{
			if (OverTime > 0.f)
				ActorIterator->SetLifeSpan(OverTime);
			else
				ActorIterator->Destroy();
		}
	}

	return EQuestActionExecuteResult::Success;
}
