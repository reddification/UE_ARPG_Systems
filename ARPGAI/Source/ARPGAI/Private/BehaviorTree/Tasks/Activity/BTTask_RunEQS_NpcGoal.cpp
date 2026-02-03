// 


#include "BehaviorTree/Tasks/Activity/BTTask_RunEQS_NpcGoal.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Interfaces/NpcGoalManager.h"

UBTTask_RunEQS_NpcGoal::UBTTask_RunEQS_NpcGoal()
{
	NodeName = "Run EQS from NPC goal";
	ResultBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RunEQS_NpcGoal, ResultBBKey), AActor::StaticClass());
	ResultBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RunEQS_NpcGoal, ResultBBKey));
	QueryFinishedDelegate = FQueryFinishedSignature::CreateUObject(this, &UBTTask_RunEQS_NpcGoal::OnQueryFinished);
}

EBTNodeResult::Type UBTTask_RunEQS_NpcGoal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!EqsId.IsValid())
		return EBTNodeResult::Failed;
	
	auto NpcGoalManager = GetNpcGoalManager(OwnerComp);
	if (!NpcGoalManager)
		return EBTNodeResult::Failed;
	
	auto EQSRequest = NpcGoalManager->GetGoalEQSRequest(EqsId);
	if (EQSRequest == nullptr)
		return EBTNodeResult::Failed;
	
	if (EQSRequest->QueryTemplate == nullptr)
		return EBTNodeResult::Failed;
	
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto BTMemory = reinterpret_cast<FBTMemory_RunActivityEQS*>(NodeMemory);
	BTMemory->RequestID = EQSRequest->Execute(*Pawn, OwnerComp.GetBlackboardComponent(), QueryFinishedDelegate);
	if (BTMemory->RequestID != INDEX_NONE)
	{
		WaitForMessage(OwnerComp, UBrainComponent::AIMessage_QueryFinished, BTMemory->RequestID);
		return EBTNodeResult::InProgress;
	}
	else
		return EBTNodeResult::Failed;
	
}

void UBTTask_RunEQS_NpcGoal::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (Result->IsAborted())
		return;

	AActor* MyOwner = Cast<AActor>(Result->Owner.Get());
	if (APawn* PawnOwner = Cast<APawn>(MyOwner))
		MyOwner = PawnOwner->GetController();

	UBehaviorTreeComponent* MyComp = MyOwner ? MyOwner->FindComponentByClass<UBehaviorTreeComponent>() : NULL;
	if (!MyComp)
	{
		UE_LOG(LogBehaviorTree, Warning, TEXT("Unable to find behavior tree to notify about finished query from %s!"), *GetNameSafe(MyOwner));
		return;
	}

	bool bSuccess = Result->IsSuccessful() && (Result->Items.Num() >= 1);
	if (bSuccess)
	{
		UBlackboardComponent* MyBlackboard = MyComp->GetBlackboardComponent();
		UEnvQueryItemType* ItemTypeCDO = Result->ItemType->GetDefaultObject<UEnvQueryItemType>();

		bSuccess = ItemTypeCDO->StoreInBlackboard(ResultBBKey, MyBlackboard, Result->RawData.GetData() + Result->Items[0].DataOffset);		
		if (!bSuccess)
		{
			UE_VLOG(MyOwner, LogBehaviorTree, Warning, TEXT("Failed to store query result! item:%s key:%s"),
				*UEnvQueryTypes::GetShortTypeName(Result->ItemType).ToString(),
				*UBehaviorTreeTypes::GetShortTypeName(ResultBBKey.SelectedKeyType));
		}
	}
	else
	{
		UBlackboardComponent* MyBlackboard = MyComp->GetBlackboardComponent();
		check(MyBlackboard);
		MyBlackboard->ClearValue(ResultBBKey.GetSelectedKeyID());
	}

	FAIMessage::Send(MyComp, FAIMessage(UBrainComponent::AIMessage_QueryFinished, this, Result->QueryID, bSuccess));
}

EBTNodeResult::Type UBTTask_RunEQS_NpcGoal::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (UEnvQueryManager* QueryManager = UEnvQueryManager::GetCurrent(OwnerComp.GetWorld()))
	{
		FBTMemory_RunActivityEQS* MyMemory = reinterpret_cast<FBTMemory_RunActivityEQS*>(NodeMemory);
		QueryManager->AbortQuery(MyMemory->RequestID);
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_RunEQS_NpcGoal::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		ResultBBKey.ResolveSelectedKey(*BB);
	}
}

void UBTTask_RunEQS_NpcGoal::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                              EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FBTMemory_RunActivityEQS>(NodeMemory, InitType);
}

void UBTTask_RunEQS_NpcGoal::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FBTMemory_RunActivityEQS>(NodeMemory, CleanupType);
}

FString UBTTask_RunEQS_NpcGoal::GetStaticDescription() const
{
	if (!EqsId.IsValid())
		return TEXT("Warning! EQS Id not set");
	
	return FString::Printf(TEXT("EQS Id: %s\n[out]Result BB: %s"), *EqsId.ToString(), *ResultBBKey.SelectedKeyName.ToString());
}

#if WITH_EDITOR

FName UBTTask_RunEQS_NpcGoal::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Task.RunEQSQuery.Icon");
}
#endif
