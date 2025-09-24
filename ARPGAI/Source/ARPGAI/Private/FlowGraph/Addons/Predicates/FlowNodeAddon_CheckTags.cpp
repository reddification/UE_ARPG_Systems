// 


#include "FlowGraph/Addons/Predicates/FlowNodeAddon_CheckTags.h"

#include "AIController.h"
#include "GameplayTagAssetInterface.h"


bool UFlowNodeAddon_CheckTags::EvaluatePredicate_Implementation() const
{
	if (QueryFilter.IsEmpty())
	{
		ensure(false);
		return true;
	}
	
	IGameplayTagAssetInterface* TagAssetInterface = nullptr;
	auto OwnerActor = TryGetRootFlowActorOwner();
	if (auto AIController = Cast<AAIController>(OwnerActor))
		TagAssetInterface = Cast<IGameplayTagAssetInterface>(AIController->GetPawn());
	else if (auto Pawn = Cast<APawn>(OwnerActor))
		TagAssetInterface = Cast<IGameplayTagAssetInterface>(Pawn);

	if (TagAssetInterface == nullptr)
		return false;

	FGameplayTagContainer PawnTags;
	TagAssetInterface->GetOwnedGameplayTags(PawnTags);
	return QueryFilter.Matches(PawnTags);
}

#if WITH_EDITOR

FText UFlowNodeAddon_CheckTags::GetNodeConfigText() const
{
	return FText::FromString(QueryFilter.IsEmpty() ? TEXT("Warning: no filter specified") : *QueryFilter.GetDescription());
}

#endif
