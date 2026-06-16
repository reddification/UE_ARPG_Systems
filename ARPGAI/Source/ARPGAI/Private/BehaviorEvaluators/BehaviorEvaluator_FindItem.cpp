#include "BehaviorEvaluators/BehaviorEvaluator_FindItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcPerceptionComponent.h"

UBehaviorEvaluatorConfig_FindItem::UBehaviorEvaluatorConfig_FindItem()
{
	FoundActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBehaviorEvaluatorConfig_FindItem, FoundActorBBKey), AActor::StaticClass());
}

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_FindItem::CreateEvaluator(UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_FindItem>(*BTComponent, this);
}

FBehaviorEvaluator_FindItem::FBehaviorEvaluator_FindItem(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* Config)
	: Super(OwnerComp, Config)
{
	FindItemConfig = Cast<UBehaviorEvaluatorConfig_FindItem>(Config);
}

void FBehaviorEvaluator_FindItem::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	AActor* BestItem = nullptr;
	float BestItemScore = 0.f;
	
	const auto& ObservedItems = PerceptionComponent->GetPerceivedValueableItems();
	const auto& PerceivedCharacters = PerceptionComponent->GetShortTermCharactersMemory();
	for (const auto& ObservedItem : ObservedItems)
	{
		if (!ObservedItem.Actor.IsValid() || !ObservedItem.ItemId.MatchesAny(FindItemConfig->ItemsIdsFilter))
			continue;
		
		float CurrentItemScore = ObservedItem.Value * FindItemConfig->ItemScoreDistanceDependency.GetRichCurveConst()->Eval(ObservedItem.Distance);
		if (FindItemConfig->bConsiderHostileActors)
		{
			for (const auto& PerceivedCharacter : PerceivedCharacters)
			{
				if (!PerceivedCharacter.Value.bHostile || !PerceivedCharacter.Value.bAlive || !PerceivedCharacter.Value.HasVisualDetection())
					continue;
				
				// 11 Jun 2026 (aki): TODO add if target is perceived hostile actor in combat check.
				// if true - item is less unfavourable, because perceived hostile character is occupied in combat
				const float DistanceToItem = (PerceivedCharacter.Key->GetActorLocation() - ObservedItem.Actor.Get()->GetActorLocation()).Size();
				
			}
		}
		
		if (CurrentItemScore > BestItemScore)
		{
			BestItemScore = CurrentItemScore;
			BestItem = ObservedItem.Actor.Get();
		}
	}
	
	if (BestItem && CachedFoundItem != BestItem)
	{
		CachedFoundItem = BestItem;
	}
}

void FBehaviorEvaluator_FindItem::OnActivated()
{
	Super::OnActivated();
	if (CachedFoundItem.IsValid())
		if (Blackboard.IsValid() && FindItemConfig.IsValid())
			Blackboard->SetValueAsObject(FindItemConfig->FoundActorBBKey.SelectedKeyName, CachedFoundItem.Get());
}

void FBehaviorEvaluator_FindItem::Cleanup()
{
	Super::Cleanup();
	if (Blackboard.IsValid() && FindItemConfig.IsValid())
		Blackboard->ClearValue(FindItemConfig->FoundActorBBKey.SelectedKeyName);
}

void FBehaviorEvaluator_FindItem::HandleMessage_Internal(const FGameplayTag& MessageTag)
{
	Super::HandleMessage_Internal(MessageTag);
	if (MessageTag == FindItemConfig->ItemNotFoundEventTag)
		ChangeUtility(-FindItemConfig->UtilityReductionOnSearchFailed);
}
