#pragma once

#include "WorldConditions/SmartObjectWorldConditionBase.h"
#include "WorldConditions/WorldCondition_SmartObjectActorTagQuery.h"

#include "WorldCondition_ActivitySmartObjectActorTagQuery.generated.h"

USTRUCT(meta=(DisplayName="Check smart object actor tags match activity requirements"))
struct ARPGAI_API FWorldCondition_ActivitySmartObjectActorTagQuery : public FSmartObjectWorldConditionBase
{
	GENERATED_BODY()

	using FStateType = FWorldCondition_SmartObjectActorTagQueryState;

protected:
#if WITH_EDITOR
	virtual FText GetDescription() const override;
#endif

	virtual TObjectPtr<const UStruct>* GetRuntimeStateType() const override
	{
		static TObjectPtr<const UStruct> Ptr{FStateType::StaticStruct()};
		return &Ptr;
	}
	
	virtual bool Initialize(const UWorldConditionSchema& Schema) override;
	virtual bool Activate(const FWorldConditionContext& Context) const override;
	virtual FWorldConditionResult IsTrue(const FWorldConditionContext& Context) const override;
	virtual void Deactivate(const FWorldConditionContext& Context) const override;

	/** Smart Object's owning actor for which the tags must match the query. The Actor must implement IGameplayTagAssetInterface or have an AbilitySystemComponent. */
	FWorldConditionContextDataRef SmartObjectActorRef;

};