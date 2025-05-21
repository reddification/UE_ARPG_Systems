
#include "SmartObjects/WorldCondition_ActivitySmartObjectActorTagQuery.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "GameplayTagAssetInterface.h"
#include "SmartObjectTypes.h"
#include "WorldConditionContext.h"
#include "WorldConditionTypes.h"
#include "Components/Controller/NpcActivityComponent.h"
#include "WorldConditions/SmartObjectWorldConditionSchema.h"

FWorldConditionResult FWorldCondition_ActivitySmartObjectActorTagQuery::IsTrue(const FWorldConditionContext& Context) const
{
	const AActor* const SmartObjectActor = Context.GetContextDataPtr<AActor>(this->SmartObjectActorRef);
	const IGameplayTagAssetInterface* GameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(SmartObjectActor);
	if (!ensure(GameplayTagAssetInterface))
	{
		return FWorldConditionResult(EWorldConditionResultValue::IsFalse, true);
	}

	FStateType& State = Context.GetState(*this);
	const bool bResultCanBeCached = State.DelegateHandle.IsValid();
	FWorldConditionResult Result(EWorldConditionResultValue::IsFalse, bResultCanBeCached);
	// Alas, this shit doesn't work. There's nowhere to take AI controller from now, until epics update this framework.
	// (at least at a considerable code complexity)
	// so currently this condition doesnt work. Instead activity smart object actor tags are checked in EnvQueryGenerator_ActivitySmartObjects
	if (auto AIController = Cast<AAIController>(Context.GetOwner()))
	{
		if (auto NpcActivityComponent = AIController->FindComponentByClass<UNpcActivityComponent>())
		{
			if (auto ActiveSmartObjectNpcGoal = Cast<UNpcGoalUseSmartObject>(NpcActivityComponent->GetActiveGoal()))
			{
				auto Parameters = ActiveSmartObjectNpcGoal->GetParameters(AIController->GetPawn());
				if (Parameters.SmartObjectActorFilter.IsEmpty())
				{
					Result.Value = EWorldConditionResultValue::IsTrue;
				}
				else
				{
					FGameplayTagContainer Tags;
					GameplayTagAssetInterface->GetOwnedGameplayTags(Tags);
					if (Parameters.SmartObjectActorFilter.Matches(Tags))
					{
						Result.Value = EWorldConditionResultValue::IsTrue;
					}	
				}
			}
		}
	}

	return Result;
}

bool FWorldCondition_ActivitySmartObjectActorTagQuery::Initialize(const UWorldConditionSchema& Schema)
{
	const USmartObjectWorldConditionSchema* SmartObjectSchema = Cast<USmartObjectWorldConditionSchema>(&Schema);
	if (SmartObjectSchema == nullptr)
	{
		UE_LOG(LogSmartObject, Error, TEXT("[%hs] Expecting schema based on %s."), __FUNCTION__, *USmartObjectWorldConditionSchema::StaticClass()->GetName());
		return false;
	}

	SmartObjectActorRef = SmartObjectSchema->GetSmartObjectActorRef();

	bCanCacheResult = Schema.GetContextDataTypeByRef(SmartObjectActorRef) == EWorldConditionContextDataType::Persistent;

	return true;
}

bool FWorldCondition_ActivitySmartObjectActorTagQuery::Activate(const FWorldConditionContext& Context) const
{
	if (!SmartObjectActorRef.IsValid())
	{
		UE_VLOG_UELOG(Context.GetOwner(), LogWorldCondition, Error, TEXT("[%s] The provided 'SmartObjectActorRef' is not set! Owner: %s ; SmartObjectActorRef: %s"),
			ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(Context.GetOwner()), *SmartObjectActorRef.GetName().ToString());
		
		return false;
	}
	
	const AActor* const SmartObjectActor = Context.GetContextDataPtr<AActor>(SmartObjectActorRef);

	if (bCanCacheResult)
	{
		if (Cast<IGameplayTagAssetInterface>(SmartObjectActor) == nullptr)
		{
			UE_VLOG_UELOG(Context.GetOwner(), LogWorldCondition, Error,
				TEXT("[%s] The provided 'SmartObjectActor' does not implement IGameplayTagAssetInterface or does not have an AbilitySystemComponent. Owner: %s ; SmartObjectActor: %s"),
				ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(Context.GetOwner()), *GetNameSafe(SmartObjectActor));
			
			return false;
		}
	}

	return true;
}

void FWorldCondition_ActivitySmartObjectActorTagQuery::Deactivate(const FWorldConditionContext& Context) const
{
	FStateType& State = Context.GetState(*this);

	if (State.DelegateHandle.IsValid())
	{
		State.DelegateHandle.Reset();
	}
}

#if WITH_EDITOR
FText FWorldCondition_ActivitySmartObjectActorTagQuery::GetDescription() const
{
	return FText::FromString("Smart object activity tags");
}
#endif