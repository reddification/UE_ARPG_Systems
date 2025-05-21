

#include "BehaviorTree/Services/BTService_RunEQS_Enhanced.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Data/LogChannels.h"
#include "VisualLogger/VisualLogger.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"

UBTService_RunEQS_Enhanced::UBTService_RunEQS_Enhanced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Run EQS query (enhanced)";
	QueryFinishedDelegate.Unbind();
	QueryFinishedDelegate = FQueryFinishedSignature::CreateUObject(this, &UBTService_RunEQS_Enhanced::OnQueryFinished2);
	bNotifyBecomeRelevant = true;
	GateBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RunEQS_Enhanced, GateBBKey));
	GateBBKey.AllowNoneAsValue(true);
}

void UBTService_RunEQS_Enhanced::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		GateBBKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_RunEQS_Enhanced::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    FBTMemory_RunEQSWithThreshold* MyMemory = reinterpret_cast<FBTMemory_RunEQSWithThreshold*>(NodeMemory);
    MyMemory->PreviousLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName);
	if (!GateBBKey.IsNone())
	{
		auto Blackboard = OwnerComp.GetBlackboardComponent();
		FOnBlackboardChangeNotification OnBlackboardChangeNotification = FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_RunEQS_Enhanced::OnGateUpdated);
		MyMemory->GateObserverDelegateHandle = Blackboard->RegisterObserver(GateBBKey.GetSelectedKeyID(), this, OnBlackboardChangeNotification);
		bool bEnableEqsTick = Blackboard->GetValueAsBool(GateBBKey.SelectedKeyName) ^ bInversedGate;
		if (!bEnableEqsTick)
			SetNextTickTime(NodeMemory, FLT_MAX);
	}
}

void UBTService_RunEQS_Enhanced::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!GateBBKey.IsNone())
	{
		if (auto Blackboard = OwnerComp.GetBlackboardComponent())
		{
		    FBTMemory_RunEQSWithThreshold* MyMemory = reinterpret_cast<FBTMemory_RunEQSWithThreshold*>(NodeMemory);
			Blackboard->UnregisterObserver(GateBBKey.GetSelectedKeyID(), MyMemory->GateObserverDelegateHandle);
		}
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_RunEQS_Enhanced::OnQueryFinished2(TSharedPtr<FEnvQueryResult> Result)
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
		UE_VLOG(BTComp->GetAIOwner(), LogARPGAI, Warning, TEXT("Unable to find behavior tree to notify about finished query from %s!"), *GetNameSafe(MyOwner));
        return;
    }

    FBTMemory_RunEQSWithThreshold* MyMemory = CastInstanceNodeMemory<FBTMemory_RunEQSWithThreshold>(BTComp->GetNodeMemory(this, BTComp->FindInstanceContainingNode(this)));
    if (!MyMemory || MyMemory->RequestID == INDEX_NONE)
	{
		UE_VLOG(BTComp->GetAIOwner(), LogARPGAI, Warning, TEXT("EQS request finished but apparently query id is invalid. Has the EQS service/task stopped working before EQS request has finished?"));
    	return;
    }
	
    MyMemory->RequestID = INDEX_NONE;

    if (Result->Items.Num() > 0)
    {
        if (MyMemory->PreviousLocation != FVector::ZeroVector && MyMemory->PreviousLocation != FAISystem::InvalidLocation)
        {
            int ThresholdCount = FMath::CeilToInt(Result->Items.Num() * ScoreUpdateThresholdCountRatio);
            ensure(Result->ItemType == UEnvQueryItemType_Point::StaticClass());
            const float BestScore = Result->Items[0].Score;
            for (int i = 0; i < ThresholdCount; i++)
            {
                FVector ItemLocation = UEnvQueryItemType_Point::GetValue(Result->RawData.GetData() + Result->Items[i].DataOffset);
                float DistSq = FVector::DistSquared(MyMemory->PreviousLocation, ItemLocation);
                if (DistSq < ThresholdDistance * ThresholdDistance && BestScore - Result->Items[i].Score < ScoreUpdateThreshold)
                {
                    UE_VLOG(MyOwner, LogEQS, Verbose, TEXT("Previous location is within best %.2f of new items and with relatively the same score as best item[best = %.2f; current = %.2f] so not updating the location"),
                        ScoreUpdateThresholdCountRatio, BestScore, Result->Items[i].Score);
                    return;
                }
            }
        }

        UBlackboardComponent* MyBlackboard = BTComp->GetBlackboardComponent();
        UEnvQueryItemType* ItemTypeCDO = Result->ItemType->GetDefaultObject<UEnvQueryItemType>();
        bool bSuccess = ItemTypeCDO->StoreInBlackboard(BlackboardKey, MyBlackboard, Result->RawData.GetData() + Result->Items[0].DataOffset);
        if (!bSuccess)
        {
            UE_VLOG(MyOwner, LogBehaviorTree, Warning, TEXT("Failed to store query result! item:%s key:%s"),
                *UEnvQueryTypes::GetShortTypeName(Result->ItemType).ToString(),
                *UBehaviorTreeTypes::GetShortTypeName(BlackboardKey.SelectedKeyType));
        }

        // MyMemory->PreviousItemScore = Result->Items[0].Score;
        MyMemory->PreviousLocation = UEnvQueryItemType_Point::GetValue(Result->RawData.GetData() + Result->Items[0].DataOffset);
    }
}

EBlackboardNotificationResult UBTService_RunEQS_Enhanced::OnGateUpdated(const UBlackboardComponent& BlackboardComponent,
	FBlackboard::FKey Key)
{
	if (ensure(Key == GateBBKey.GetSelectedKeyID()))
	{
		bool bRunService = BlackboardComponent.GetValue<UBlackboardKeyType_Bool>(Key) ^ bInversedGate;
		auto BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BlackboardComponent.GetBrainComponent());
		auto NodeMemory = BehaviorTreeComponent->GetNodeMemory(this, BehaviorTreeComponent->FindInstanceContainingNode(this));
		if (bRunService)
			ScheduleNextTick(*BehaviorTreeComponent, NodeMemory);
		else
			SetNextTickTime(NodeMemory, FLT_MAX);
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

FString UBTService_RunEQS_Enhanced::GetStaticDescription() const
{
	FString Result = Super::GetStaticDescription();

	for (const auto& QueryConfigItem : EQSRequest.QueryConfig)
	{
		if (QueryConfigItem.BBKey.IsSet())
		{
			Result = Result.Append(FString::Printf(TEXT("\n%s = %s"), *QueryConfigItem.ParamName.ToString(), *QueryConfigItem.BBKey.SelectedKeyName.ToString()));
		}
		else
		{
			Result = Result.Append(FString::Printf(TEXT("\n%s = %.2f"), *QueryConfigItem.ParamName.ToString(), QueryConfigItem.Value));
		}
	}

	Result = Result.Append(FString::Printf(TEXT("\nRun Mode: %s"), *UEnum::GetDisplayValueAsText<EEnvQueryRunMode::Type>(EQSRequest.RunMode).ToString()));
	if (!GateBBKey.IsNone())
		Result = Result.Append(FString::Printf(TEXT("\nUse gate: %s%s"), *GateBBKey.SelectedKeyName.ToString(), bInversedGate ? TEXT(", inversed") : TEXT("")));
	
	return Result;
}

#if WITH_EDITOR

void UBTService_RunEQS_Enhanced::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty &&
		PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UBTService_RunEQS_Enhanced, EQSRequest))
	{
		for (const FName& CustomParamName : CustomEqsParamsNames)
		{
			TryAddCustomParamToEQS(CustomParamName);
		}

		for (const auto& CustomParamTagName : CustomEqsParamsTagNames)
		{
			auto TagName = CustomParamTagName.GetTagName();
			TryAddCustomParamToEQS(TagName);
		}
	}
}

bool UBTService_RunEQS_Enhanced::TryAddCustomParamToEQS(const FName& CustomParamName)
{
	if (EQSRequest.QueryConfig.ContainsByPredicate([CustomParamName] (const FAIDynamicParam& DynamicParam){ return DynamicParam.ParamName == CustomParamName; }))
		return true;
			
	FAIDynamicParam CustomParam = FAIDynamicParam();
	CustomParam.ParamName = CustomParamName;
	CustomParam.ParamType = EAIParamType::Float; // only float for now
	CustomParam.Value = 0.f;
	CustomParam.ConfigureBBKey(*this);

	EQSRequest.QueryConfig.Add(CustomParam);
	return false;
}

#endif