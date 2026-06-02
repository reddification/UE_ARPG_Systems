#include "BehaviorTree/Decorators/BTDecorator_TrackActorNotDestroyed.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interfaces/NpcMeaningfulActorInterface.h"

UBTDecorator_TrackActorNotDestroyed::UBTDecorator_TrackActorNotDestroyed()
{
	NodeName = "Track actor not destroyed";
	ActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_TrackActorNotDestroyed, ActorBBKey), AActor::StaticClass());
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

void UBTDecorator_TrackActorNotDestroyed::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	auto NotificationDelegate = FOnBlackboardChangeNotification::CreateUObject(this, &UBTDecorator_TrackActorNotDestroyed::OnActorChanged);
	auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent();
	Blackboard->RegisterObserver(ActorBBKey.GetSelectedKeyID(), this, NotificationDelegate);
	auto CurrentActor = Cast<AActor>(Blackboard->GetValueAsObject(ActorBBKey.SelectedKeyName));
	auto BTMemory = GetNodeMemory<FBTMemory_TrackActorNotDestroyed>(SearchData);
	ensure(!BTMemory->CurrentActor.IsValid());
	if (CurrentActor)
	{
		if (auto TrackDestroyInterface = Cast<INpcMeaningfulActorInterface>(CurrentActor))
		{
			BTMemory->CurrentActor = CurrentActor;
			TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerBT = &SearchData.OwnerComp;
			BTMemory->DestroyDelegateHandle = TrackDestroyInterface->NpcMeaningfulActorDestroyedEvent.AddUObject(this,
				&UBTDecorator_TrackActorNotDestroyed::OnActorDestroyed, WeakOwnerBT);
		}
	}
}

void UBTDecorator_TrackActorNotDestroyed::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	if (auto Blackboard = SearchData.OwnerComp.GetBlackboardComponent())
		Blackboard->UnregisterObserversFrom(this);

	if (auto BTMemory = GetNodeMemory<FBTMemory_TrackActorNotDestroyed>(SearchData))
		ResetCurrentActor(BTMemory);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_TrackActorNotDestroyed::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	InitializeNodeMemory<FBTMemory_TrackActorNotDestroyed>(NodeMemory, InitType);
}

void UBTDecorator_TrackActorNotDestroyed::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTMemory_TrackActorNotDestroyed>(NodeMemory, CleanupType);
}

void UBTDecorator_TrackActorNotDestroyed::ResetCurrentActor(FBTMemory_TrackActorNotDestroyed* BTMemory)
{
	if (BTMemory->CurrentActor.IsValid() && ensure(BTMemory->DestroyDelegateHandle.IsValid()))
	{
		if (auto CurrentInterface = Cast<INpcMeaningfulActorInterface>(BTMemory->CurrentActor.Get()))
			CurrentInterface->NpcMeaningfulActorDestroyedEvent.Remove(BTMemory->DestroyDelegateHandle);
		
		BTMemory->CurrentActor.Reset();
		BTMemory->DestroyDelegateHandle.Reset();
	}
}

EBlackboardNotificationResult UBTDecorator_TrackActorNotDestroyed::OnActorChanged(
	const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key)
{
	UBehaviorTreeComponent* BehaviorComp = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());

	auto* BTMemory = reinterpret_cast<FBTMemory_TrackActorNotDestroyed*>(BehaviorComp->GetNodeMemory(this,
		BehaviorComp->FindInstanceContainingNode(this)));
	
	ResetCurrentActor(BTMemory);

	if (auto NewActor = Cast<AActor>(BlackboardComponent.GetValueAsObject(ActorBBKey.SelectedKeyName)))
	{
		if (auto TrackDestroyInterface = Cast<INpcMeaningfulActorInterface>(NewActor))
		{
			BTMemory->CurrentActor = NewActor;
			TWeakObjectPtr WeakOwnerBT = BehaviorComp;
			BTMemory->DestroyDelegateHandle = TrackDestroyInterface->NpcMeaningfulActorDestroyedEvent.AddUObject(this,
				&UBTDecorator_TrackActorNotDestroyed::OnActorDestroyed, WeakOwnerBT);
		}
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}

void UBTDecorator_TrackActorNotDestroyed::OnActorDestroyed(AActor* Actor, AActor* Instigator, TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerBT)
{
	if (!WeakOwnerBT.IsValid())
		return;
	
	auto* BTMemory = reinterpret_cast<FBTMemory_TrackActorNotDestroyed*>(WeakOwnerBT->GetNodeMemory(this,
		WeakOwnerBT->FindInstanceContainingNode(this)));
	if (ensure(BTMemory->CurrentActor == Actor))
	{
		ResetCurrentActor(BTMemory);
		WeakOwnerBT->GetBlackboardComponent()->ClearValue(ActorBBKey.SelectedKeyName);
	}
}

void UBTDecorator_TrackActorNotDestroyed::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
		ActorBBKey.ResolveSelectedKey(*BB);
}

FString UBTDecorator_TrackActorNotDestroyed::GetStaticDescription() const
{
	return FString::Printf(TEXT("Clear %s when actor is destroyed"), *ActorBBKey.SelectedKeyName.ToString());
}
