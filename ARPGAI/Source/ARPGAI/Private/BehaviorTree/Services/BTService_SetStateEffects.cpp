

#include "BehaviorTree/Services/BTService_SetStateEffects.h"

#include "AIHelpers.h"
#include "BlackboardKeyType_GameplayTag.h"
#include "Activities/ActivityInstancesHelper.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/LogChannels.h"

UBTService_SetStateEffects::UBTService_SetStateEffects()
{
	NodeName = "Set state effects";
	bNotifyBecomeRelevant = 1;
	bNotifyCeaseRelevant = 1;
	bNotifyTick = true;
	NewStateTagBBKey.AllowedTypes.Add(NewObject<UBlackboardKeyType_GameplayTag>(this, GET_MEMBER_NAME_CHECKED(UBTService_SetStateEffects, NewStateTagBBKey)));
	NewStateTagBBKey.AllowNoneAsValue(true);
}

void UBTService_SetStateEffects::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	ensure(ActivationDelay > 0.f);
	if (auto NpcComponent = GetNpcComponent(OwnerComp))
	{
		FGameplayTag ActualStateTag = GetActualStateTag(OwnerComp);
		if (ActualStateTag.IsValid())
		{
			UE_VLOG(OwnerComp.GetAIOwner(), LogARPGAI_NpcStates, Verbose, TEXT("Activating delayed state %s"), *ActualStateTag.ToString());
			NpcComponent->SetStateActive(NewStateTag, SetByCallerParams, true);
		}
	}

	SetNextTickTime(NodeMemory, FLT_MAX);
}

void UBTService_SetStateEffects::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	if (ActivationDelay > 0.f)
	{
		Interval = ActivationDelay;
		ScheduleNextTick(OwnerComp, NodeMemory);
	}
	else if (auto NpcComponent = GetNpcComponent(OwnerComp))
	{
		FGameplayTag ActualStateTag = GetActualStateTag(OwnerComp);
		if (ActualStateTag.IsValid())
			NpcComponent->SetStateActive(ActualStateTag, SetByCallerParams, true);
		
		SetNextTickTime(NodeMemory, FLT_MAX);
	}
}

void UBTService_SetStateEffects::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FGameplayTag ActualStateTag = GetActualStateTag(OwnerComp);
	if (ActualStateTag.IsValid())
	{
		if (auto NpcComponent = GetNpcComponent(OwnerComp))
		{
			NpcComponent->SetStateActive(ActualStateTag, SetByCallerParams, false);
		}
	}

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_SetStateEffects::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BBAsset = Asset.GetBlackboardAsset())
	{
		NewStateTagBBKey.ResolveSelectedKey(*BBAsset);
	}
}

FGameplayTag UBTService_SetStateEffects::GetActualStateTag(UBehaviorTreeComponent& OwnerComp) const
{
	if (NewStateTagBBKey.IsNone())
		return NewStateTag;

	FGameplayTagContainer BlackboardStateTag = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_GameplayTag>(NewStateTagBBKey.GetSelectedKeyID());
	return BlackboardStateTag.IsEmpty() ? NewStateTag : BlackboardStateTag.First();
}

FString UBTService_SetStateEffects::GetStaticDescription() const
{
	FString Result = NewStateTagBBKey.IsNone() ? NewStateTag.ToString() : NewStateTagBBKey.SelectedKeyName.ToString();
	if (!SetByCallerParams.IsEmpty())
	{
		Result = Result.Append(TEXT("Set by caller params:"));
		for (const auto& SetByCallerParam : SetByCallerParams)
		{
			Result = Result.Append(FString::Printf(TEXT("\n%s: %.2f"), *SetByCallerParam.Key.ToString(), SetByCallerParam.Value));
		}
	}
	return ActivationDelay > 0.f
		? Result.Append(FString::Printf(TEXT("\nActivate with %.2fs delay"), ActivationDelay))
		: Result;
}
