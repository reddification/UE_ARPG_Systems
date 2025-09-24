#include "BehaviorTree/Tasks/SmartObjects/BTTask_FindAndClaimActivitySmartObject.h"

#include "AIController.h"
#include "BlackboardKeyType_SOClaimHandle.h"
#include "EnvQueryItemType_SmartObject.h"
#include "SmartObjectComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AiDataTypes.h"
#include "EnvironmentQuery/EnvQueryManager.h"

UBTTask_FindAndClaimActivitySmartObject::UBTTask_FindAndClaimActivitySmartObject()
{
	NodeName = "Find and claim Activity Smart Object";
	InteractionActorBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindAndClaimActivitySmartObject, InteractionActorBBKey), AActor::StaticClass());
	OutSmartObjectSlotLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindAndClaimActivitySmartObject, OutSmartObjectSlotLocationBBKey));
	OutSmartObjectSlotRotationBBKey.AddRotatorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindAndClaimActivitySmartObject, OutSmartObjectSlotRotationBBKey));
	OutSmartObjectClaimHandleBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_SOClaimHandle>(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindAndClaimActivitySmartObject, OutSmartObjectClaimHandleBBKey)));
	EQSQueryFinishedDelegate = FQueryFinishedSignature::CreateUObject(this, &UBTTask_FindAndClaimActivitySmartObject::OnQueryFinished);
	EQSRequest.RunMode = EEnvQueryRunMode::RandomBest5Pct;
	EQSRunModeBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindAndClaimActivitySmartObject, EQSRunModeBBKey), StaticEnum<EEnvQueryRunMode::Type>());
	// OutAttackResultBBKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, OutAttackResultBBKey), StaticEnum<ENpcAttackResult>());
	
}

// TODO smart object claim handle processing is scattered across NpcActivityInstance and BTTasks. The logic needs to be consolidated in 1 place
EBTNodeResult::Type UBTTask_FindAndClaimActivitySmartObject::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTTask_FindAndClaimActivitySmartObject::ExecuteTask)

	// auto NpcActivityComponent = GetNpcActivityComponent(OwnerComp);
	// if (NpcActivityComponent == nullptr)
	// 	return EBTNodeResult::Failed;
	
	AAIController* MyController = OwnerComp.GetAIOwner();
	APawn* Pawn = MyController->GetPawn();
	
	// if there's no SOCH - run an EQS to find one
	FBTMemory_FindAndClaimSmartObject* MyMemory = reinterpret_cast<FBTMemory_FindAndClaimSmartObject*>(NodeMemory);
	MyMemory->EQSRequestID = INDEX_NONE;
	if (EQSRequest.IsValid() && (EQSRequest.EQSQueryBlackboardKey.IsSet() || EQSRequest.QueryTemplate))
	{
		const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
		auto InitialRunMode = EQSRequest.RunMode;
		if (!EQSRunModeBBKey.SelectedKeyName.IsNone())
			EQSRequest.RunMode = static_cast<EEnvQueryRunMode::Type>(BlackboardComponent->GetValueAsEnum(EQSRunModeBBKey.SelectedKeyName));
		
		MyMemory->EQSRequestID = EQSRequest.Execute(*Pawn, BlackboardComponent, EQSQueryFinishedDelegate);
		EQSRequest.RunMode = InitialRunMode;
		
		if (MyMemory->EQSRequestID != INDEX_NONE)
			return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_FindAndClaimActivitySmartObject::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTMemory_FindAndClaimSmartObject* MyMemory = reinterpret_cast<FBTMemory_FindAndClaimSmartObject*>(NodeMemory);

	if (MyMemory->EQSRequestID != INDEX_NONE)
	{
		if (UWorld* World = OwnerComp.GetWorld())
			if (UEnvQueryManager* EnvQueryManager = UEnvQueryManager::GetCurrent(World))
				EnvQueryManager->AbortQuery(MyMemory->EQSRequestID);
		
		MyMemory->EQSRequestID = INDEX_NONE;
	}
	
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_FindAndClaimActivitySmartObject::ClaimSmartObject(UBehaviorTreeComponent* BTComponent, USmartObjectSubsystem* SmartObjectSubsystem, const FSmartObjectClaimHandle ClaimHandle)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTTask_FindAndClaimActivitySmartObject::ClaimSmartObject)
	
	AActor* SmartObjectActor = SmartObjectSubsystem->GetSmartObjectComponent(ClaimHandle)->GetOwner();
					
	auto Blackboard = BTComponent->GetBlackboardComponent();
	Blackboard->SetValueAsObject(InteractionActorBBKey.SelectedKeyName, SmartObjectActor);
	Blackboard->SetValue<UBlackboardKeyType_SOClaimHandle>(OutSmartObjectClaimHandleBBKey.SelectedKeyName, ClaimHandle);
	
	FTransform SlotTransform;
	SmartObjectSubsystem->GetSlotTransform(ClaimHandle, SlotTransform);
	Blackboard->SetValueAsVector(OutSmartObjectSlotLocationBBKey.SelectedKeyName, SlotTransform.GetLocation());
	Blackboard->SetValueAsRotator(OutSmartObjectSlotRotationBBKey.SelectedKeyName, SlotTransform.GetRotation().Rotator());

	SmartObjectSubsystem->AddSlotData(ClaimHandle, FConstStructView::Make(FActivitySmartObjectSlotUserData(BTComponent->GetAIOwner()->GetPawn())));
}

void UBTTask_FindAndClaimActivitySmartObject::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UBTTask_FindAndClaimActivitySmartObject::OnQueryFinished)
	
	if (!Result)
	{
		return;
	}

	AActor* MyOwner = Cast<AActor>(Result->Owner.Get());
	if (APawn* PawnOwner = Cast<APawn>(MyOwner))
	{
		MyOwner = PawnOwner->GetController();
	}

	UBehaviorTreeComponent* BTComponent = MyOwner ? MyOwner->FindComponentByClass<UBehaviorTreeComponent>() : NULL;
	if (!ensure(BTComponent)) return;
	
	const FEnvQueryResult& QueryResult = *Result.Get();

	uint8* RawMemory = BTComponent->GetNodeMemory(this, BTComponent->FindInstanceContainingNode(this));
	check(RawMemory);
	FBTMemory_FindAndClaimSmartObject* MyMemory = reinterpret_cast<FBTMemory_FindAndClaimSmartObject*>(RawMemory);
	if (MyMemory->EQSRequestID != QueryResult.QueryID)
	{
		// ignoring EQS result due to QueryID mismatch.
		ensure(MyMemory->EQSRequestID != INDEX_NONE);
		return;
	}
	else if (QueryResult.IsAborted())
	{
		// observed EQS query finished as Aborted. Aborting the BT node as well.
		FinishLatentTask(*BTComponent, EBTNodeResult::Aborted);
		return;
	}

	// at this point we've already confirmed that QueryResult does indeed corresponds to the query we're waiting for
	// so we need to clear the EQSRequestID here in case the next task we're about to issue (the UAITask_UseGameplayBehaviorSmartObject)
	// might fail instantly and we do check EQSRequestID on tasks end to make sure everything has been cleaned up properly.
	MyMemory->EQSRequestID = INDEX_NONE;

	bool bSmartObjectClaimed = false;

	if (QueryResult.IsSuccessful() && (QueryResult.Items.Num() >= 1))
	{
		if (QueryResult.ItemType->IsChildOf(UEnvQueryItemType_SmartObject::StaticClass()) == false)
		{
			UE_VLOG_UELOG(BTComponent, LogSmartObject, Error, TEXT("%s used EQS query that did not generate EnvQueryItemType_SmartObject items"), *GetNodeName());
		}
		else if (USmartObjectSubsystem* SmartObjectSubsystem = USmartObjectSubsystem::GetCurrent(MyOwner->GetWorld()))
		{
			const FSmartObjectActorUserData ActorUserData(Cast<AActor>(Result->Owner.Get()));
			const FConstStructView ActorUserDataView(FConstStructView::Make(ActorUserData));

			// we could use QueryResult.GetItemAsTypeChecked, but the below implementation is more efficient
			for (int i = 0; i < QueryResult.Items.Num(); ++i)
			{
				const FSmartObjectSlotEQSItem& Item = UEnvQueryItemType_SmartObject::GetValue(QueryResult.GetItemRawMemory(i));

				const FSmartObjectClaimHandle ClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(Item.SlotHandle, ESmartObjectClaimPriority::Normal,
					ActorUserDataView);
				if (ClaimHandle.IsValid())
				{
					UE_VLOG_UELOG(BTComponent, LogSmartObject, Verbose, TEXT("%s claimed EQS-found smart object: %s"), *GetNodeName(), *LexToString(ClaimHandle));

					bSmartObjectClaimed = true;
					ClaimSmartObject(BTComponent, SmartObjectSubsystem, ClaimHandle);
					break;
				}
			}
		}
	}
	
	FinishLatentTask(*BTComponent, bSmartObjectClaimed ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

FString UBTTask_FindAndClaimActivitySmartObject::GetStaticDescription() const
{
	return FString::Printf(TEXT("[out]Interaction actor: %s\n[out]Location key: %s\n[out]Rotation key: %s\n[out]Claimed SOCH BB: %s"),
		*InteractionActorBBKey.SelectedKeyName.ToString(), *OutSmartObjectSlotLocationBBKey.SelectedKeyName.ToString(),
		*OutSmartObjectSlotRotationBBKey.SelectedKeyName.ToString(), *OutSmartObjectClaimHandleBBKey.SelectedKeyName.ToString());
}

void UBTTask_FindAndClaimActivitySmartObject::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	EQSRequest.InitForOwnerAndBlackboard(*this, GetBlackboardAsset());
}

