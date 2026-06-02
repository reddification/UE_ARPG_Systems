#include "BehaviorTree/Decorators/BTDecorator_TrackActorTags.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interfaces/NpcActorTagsInterface.h"

UBTDecorator_TrackActorTags::UBTDecorator_TrackActorTags()
{
	NodeName = "Track actor tags";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
	ActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_TrackActorTags, ActorBBKey), AActor::StaticClass());
	OutTagsBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_TrackActorTags, OutTagsBBKey)));
}

void UBTDecorator_TrackActorTags::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	
	if (AActor* CurrentActor = Cast<AActor>(Blackboard->GetValueAsObject(ActorBBKey.SelectedKeyName)))
	{
		if (INpcActorTagsInterface* TagsInterface = Cast<INpcActorTagsInterface>(CurrentActor))
		{
			FBTMemory_TrackActorTags* BTMemory = GetNodeMemory<FBTMemory_TrackActorTags>(SearchData);
			TrackNewActor(&SearchData.OwnerComp, Blackboard, CurrentActor, TagsInterface, BTMemory);
		}
	}
	
	auto Delegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_TrackActorTags::OnActorChanged);
	Blackboard->RegisterObserver(ActorBBKey.GetSelectedKeyID(), this, Delegate);
}

void UBTDecorator_TrackActorTags::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent())
	{
		if (auto BTMemory = GetNodeMemory<FBTMemory_TrackActorTags>(SearchData))
		{
			if (BTMemory->DelegateHandle.IsValid())
			{
				if (BTMemory->CurrentActor.IsValid())
				{
					auto TagsOwner = Cast<INpcActorTagsInterface>(BTMemory->CurrentActor.Get());
					TagsOwner->OnTagsChangedEvent_NPC.Remove(BTMemory->DelegateHandle);
					BTMemory->CurrentActor.Reset();
				}
				
				BTMemory->DelegateHandle.Reset();
			}
		}
		
		Blackboard->ClearValue(OutTagsBBKey.SelectedKeyName);
		Blackboard->UnregisterObserversFrom(this);	
	}
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_TrackActorTags::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	InitializeNodeMemory<FBTMemory_TrackActorTags>(NodeMemory, InitType);
}

void UBTDecorator_TrackActorTags::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTMemory_TrackActorTags>(NodeMemory, CleanupType);
}

EBlackboardNotificationResult UBTDecorator_TrackActorTags::OnActorChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	if (!ensure(Key == ActorBBKey.GetSelectedKeyID()))
		return EBlackboardNotificationResult::RemoveObserver;
	
	auto BTComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
	auto BlackboardMutable = BTComponent->GetBlackboardComponent();
	auto BTMemory = CastInstanceNodeMemory<FBTMemory_TrackActorTags>(BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this)));
	
	if (BTMemory->DelegateHandle.IsValid())
	{
		if (BTMemory->CurrentActor.IsValid())
		{
			auto TagsInterface = Cast<INpcActorTagsInterface>(BTMemory->CurrentActor.Get());
			TagsInterface->OnTagsChangedEvent_NPC.Remove(BTMemory->DelegateHandle);
			BTMemory->CurrentActor.Reset();
		}
		
		BTMemory->DelegateHandle.Reset();
	}
	
	bool bActorUpdated = false;
	if (auto NewActor = Cast<AActor>(BlackboardComponent.GetValueAsObject(ActorBBKey.SelectedKeyName)))
	{
		if (auto TagsInterface = Cast<INpcActorTagsInterface>(NewActor))
		{
			TrackNewActor(BTComponent, BlackboardMutable, NewActor, TagsInterface, BTMemory);
			bActorUpdated = true;
		}
		
	}
	if (!bActorUpdated)
		BlackboardMutable->ClearValue(OutTagsBBKey.SelectedKeyName);
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_TrackActorTags::OnActorTagsChanged(AActor* Actor, const FGameplayTagContainer& NewTags,
	TWeakObjectPtr<UBehaviorTreeComponent> BTComponent)
{
	if (!ensure(BTComponent.IsValid()))
		return;
	
	auto Blackboard = BTComponent->GetBlackboardComponent();
	Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(OutTagsBBKey.SelectedKeyName, NewTags);
}

void UBTDecorator_TrackActorTags::TrackNewActor(TWeakObjectPtr<UBehaviorTreeComponent> BTComponent, UBlackboardComponent* Blackboard,
	AActor* CurrentActor, INpcActorTagsInterface* TagsInterface, FBTMemory_TrackActorTags* BTMemory)
{
	FGameplayTagContainer CurrentTags = TagsInterface->GetTags_NPC();
	Blackboard->SetValue<UBlackboardKeyType_GameplayTag>(OutTagsBBKey.SelectedKeyName, CurrentTags);
	BTMemory->CurrentActor = CurrentActor;
	BTMemory->DelegateHandle = TagsInterface->OnTagsChangedEvent_NPC.AddUObject(this, &UBTDecorator_TrackActorTags::OnActorTagsChanged, BTComponent);
}

void UBTDecorator_TrackActorTags::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		ActorBBKey.ResolveSelectedKey(*BB);
		OutTagsBBKey.ResolveSelectedKey(*BB);
	}
}

FString UBTDecorator_TrackActorTags::GetStaticDescription() const
{
	return FString::Printf(TEXT("Track %s tags into\n%s"), *ActorBBKey.SelectedKeyName.ToString(), 
		*OutTagsBBKey.SelectedKeyName.ToString());
}
