


#include "BehaviorTree/Decorators/BTDecorator_CooldownWithDeviation.h"

UBTDecorator_CooldownWithDeviation::UBTDecorator_CooldownWithDeviation()
{
	NodeName = "Cooldown (with deviation)";
	DeviationTime = 1.f;
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;
}

bool UBTDecorator_CooldownWithDeviation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
                                                                    uint8* NodeMemory) const
{
	FBTCooldownWithDeviationMemory* DecoratorMemory = CastInstanceNodeMemory<FBTCooldownWithDeviationMemory>(NodeMemory);
	const float TimePassed = (OwnerComp.GetWorld()->GetTimeSeconds() - DecoratorMemory->LastUseTimestamp);
	return TimePassed >= DecoratorMemory->ActualCoolDownTime;
}

void UBTDecorator_CooldownWithDeviation::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
	EBTNodeResult::Type NodeResult)
{
	Super::OnNodeDeactivation(SearchData, NodeResult);
	FBTCooldownWithDeviationMemory* DecoratorMemory = GetNodeMemory<FBTCooldownWithDeviationMemory>(SearchData);
	DecoratorMemory->ActualCoolDownTime = FMath::Max(0.f,CoolDownTime.GetValue(SearchData.OwnerComp) + FMath::FRandRange(-DeviationTime, DeviationTime));
}

void UBTDecorator_CooldownWithDeviation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                  float DeltaSeconds)
{
	FBTCooldownWithDeviationMemory* DecoratorMemory = CastInstanceNodeMemory<FBTCooldownWithDeviationMemory>(NodeMemory);
	if (!DecoratorMemory->bRequestedRestart)
	{
		const float TimePassed = (OwnerComp.GetWorld()->GetTimeSeconds() - DecoratorMemory->LastUseTimestamp);
		if (TimePassed >= DecoratorMemory->ActualCoolDownTime)
		{
			DecoratorMemory->bRequestedRestart = true;
			OwnerComp.RequestExecution(this);
		}
	}
}

uint16 UBTDecorator_CooldownWithDeviation::GetInstanceMemorySize() const
{
	return sizeof(FBTCooldownWithDeviationMemory);
}

FString UBTDecorator_CooldownWithDeviation::GetStaticDescription() const
{
	return FString::Printf(TEXT("lock for %s +- %.2fs after execution and return %s"),
		*CoolDownTime.ToString(), DeviationTime, *UBehaviorTreeTypes::DescribeNodeResult(EBTNodeResult::Failed));
}

void UBTDecorator_CooldownWithDeviation::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp,
                                                               uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	FBTCooldownWithDeviationMemory* DecoratorMemory = CastInstanceNodeMemory<FBTCooldownWithDeviationMemory>(NodeMemory);
	const float TimePassed = OwnerComp.GetWorld()->GetTimeSeconds() - DecoratorMemory->LastUseTimestamp;
	
	if (TimePassed < DecoratorMemory->ActualCoolDownTime)
	{
		Values.Add(FString::Printf(TEXT("%s in %ss"),
			(FlowAbortMode == EBTFlowAbortMode::None) ? TEXT("unlock") : TEXT("restart"),
			*FString::SanitizeFloat(DecoratorMemory->ActualCoolDownTime - TimePassed)));
	}
}

void UBTDecorator_CooldownWithDeviation::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTMemoryInit::Type InitType) const
{
	Super::InitializeMemory(OwnerComp, NodeMemory, InitType);

	FBTCooldownWithDeviationMemory* DecoratorMemory = CastInstanceNodeMemory<FBTCooldownWithDeviationMemory>(NodeMemory);
	DecoratorMemory->ActualCoolDownTime = 0.f;
}
