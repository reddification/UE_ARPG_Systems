// 


#include "BehaviorTree/Services/BTService_RunEQS_NpcGoal.h"

#include "AIController.h"
#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Interfaces/NpcGoalManager.h"

UBTService_RunEQS_NpcGoal::UBTService_RunEQS_NpcGoal()
{
	NodeName = "Run NPC goal EQS query";

	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = true;
	
#if WITH_EDITORONLY_DATA
	// Do not expose the option to tick on search start since the request is async and it
	// requires the node to be relevant to properly manage the request and its delegate.
	bCanTickOnSearchStartBeExposed = false;
	bCallTickOnSearchStart = false;
#endif // WITH_EDITORONLY_DATA
	
	// accept only actors and vectors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RunEQS_NpcGoal, BlackboardKey), AActor::StaticClass());
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RunEQS_NpcGoal, BlackboardKey));

	QueryFinishedDelegate = FQueryFinishedSignature::CreateUObject(this, &UBTService_RunEQS_NpcGoal::OnQueryFinished);
}

void UBTService_RunEQS_NpcGoal::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if (!EqsId.IsValid())
		return;
	
	auto NpcGoalManager = GetNpcGoalManager(OwnerComp);
	if (!NpcGoalManager)
		return;
	
	auto EQSRequest = NpcGoalManager->GetGoalEQSRequest(EqsId);
	if (EQSRequest == nullptr)
		return ;
	
	if (EQSRequest->QueryTemplate == nullptr)
		return;
	
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto BTMemory = reinterpret_cast<FBTEQSServiceMemory*>(NodeMemory);
	BTMemory->RequestID = EQSRequest->Execute(*Pawn, OwnerComp.GetBlackboardComponent(), QueryFinishedDelegate);
	ensure(BTMemory->RequestID != INDEX_NONE);
}

void UBTService_RunEQS_NpcGoal::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTEQSServiceMemory* MyMemory = CastInstanceNodeMemory<FBTEQSServiceMemory>(NodeMemory);
	if (MyMemory && MyMemory->RequestID != INDEX_NONE)
	{
		if (UWorld* World = OwnerComp.GetWorld())
			if (UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(World))
				EQSManager->AbortQuery(MyMemory->RequestID);

		MyMemory->RequestID = INDEX_NONE;
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_RunEQS_NpcGoal::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);
	FBTEQSServiceMemory* MyMemory = InitializeNodeMemory<FBTEQSServiceMemory>(NodeMemory, InitType);
	check(MyMemory);
	MyMemory->RequestID = INDEX_NONE;
}

void UBTService_RunEQS_NpcGoal::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryClear::Type CleanupType) const
{
	Super::CleanupMemory(OwnerComp, NodeMemory, CleanupType);
	CleanupNodeMemory<FBTEQSServiceMemory>(NodeMemory, CleanupType);
}

FString UBTService_RunEQS_NpcGoal::GetStaticDescription() const
{
	if (!EqsId.IsValid())
		return TEXT("Warning! EQS Id not set");
	
	return FString::Printf(TEXT("EQS Id: %s\n[out]Result BB: %s\n%s"), *EqsId.ToString(), *BlackboardKey.SelectedKeyName.ToString(), 
		*Super::GetStaticDescription());
}


void UBTService_RunEQS_NpcGoal::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (Result->IsAborted())
	{
		return;
	}

	AActor* MyOwner = Cast<AActor>(Result->Owner.Get());
	if (APawn* PawnOwner = Cast<APawn>(MyOwner))
	{
		MyOwner = PawnOwner->GetController();
	}

	UBehaviorTreeComponent* BTComp = MyOwner ? MyOwner->FindComponentByClass<UBehaviorTreeComponent>() : NULL;
	if (!BTComp)
	{
		UE_LOG(LogBehaviorTree, Warning, TEXT("Unable to find behavior tree to notify about finished query from %s!"), *GetNameSafe(MyOwner));
		return;
	}

	FBTEQSServiceMemory* MyMemory = CastInstanceNodeMemory<FBTEQSServiceMemory>(BTComp->GetNodeMemory(this, BTComp->FindInstanceContainingNode(this)));
	if (!ensureMsgf(MyMemory && MyMemory->RequestID != INDEX_NONE, TEXT("%hs called while the BT node is not or no longer active."), __FUNCTION__))
	{
		return;
	}

	bool bSuccess = Result->IsSuccessful() && (Result->Items.Num() >= 1);
	if (bSuccess)
	{
		UBlackboardComponent* MyBlackboard = BTComp->GetBlackboardComponent();
		UEnvQueryItemType* ItemTypeCDO = Result->ItemType->GetDefaultObject<UEnvQueryItemType>();

		bSuccess = ItemTypeCDO->StoreInBlackboard(BlackboardKey, MyBlackboard, Result->RawData.GetData() + Result->Items[0].DataOffset);
		if (!bSuccess)
		{
			UE_VLOG(MyOwner, LogBehaviorTree, Warning, TEXT("Failed to store query result! item:%s key:%s"),
				*UEnvQueryTypes::GetShortTypeName(Result->ItemType).ToString(),
				*UBehaviorTreeTypes::GetShortTypeName(BlackboardKey.SelectedKeyType));
		}
	}
	else
	{
		UBlackboardComponent* MyBlackboard = BTComp->GetBlackboardComponent();
		check(MyBlackboard);
		MyBlackboard->ClearValue(BlackboardKey.GetSelectedKeyID());
	}

	MyMemory->RequestID = INDEX_NONE;
}
