

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_SetBlackboardValue.generated.h"

UENUM()
enum class EBlackboardKeyDataType : uint8
{
	Float,
	Int,
	Bool,
	String,
	GameplayTag,
	Vector
};

// Experimental!
UCLASS(HideCategories=(FlowControl,Condition))
class ARPGAI_API UBTDecorator_SetBlackboardValue : public UBTDecorator
{
	GENERATED_BODY()

private:
	struct FSetBlackboardValueMemory
	{
		float InitialFloatValue = 0.f;
		int32 InitialIntValue = 0;
		FString InitialStringValue;
		FName InitialNameValue = NAME_None;
		FVector InitialVectorValue = FVector::ZeroVector;
		bool InitialBoolValue = false;
		bool bWasSet = false;
	};
	
public:
	UBTDecorator_SetBlackboardValue();
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
protected:
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "BlackboardKeyDataType == EBlackboardKeyDataType::Float"))
	float FloatValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "BlackboardKeyDataType == EBlackboardKeyDataType::Bool"))
	bool BoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "BlackboardKeyDataType == EBlackboardKeyDataType::Int"))
	int32 IntValue = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "BlackboardKeyDataType == EBlackboardKeyDataType::String"))
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "BlackboardKeyDataType == EBlackboardKeyDataType::GameplayTag"))
	FGameplayTag TagValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "BlackboardKeyDataType == EBlackboardKeyDataType::Vector"))
	FVector VectorValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBlackboardKeyDataType BlackboardKeyDataType = EBlackboardKeyDataType::Float;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector BlackboardKey;	
	
private:
	void SetBlackboardValue(UBlackboardComponent* BlackboardComponent, FSetBlackboardValueMemory* NodeMemory, bool bApply);

	template<class TDataClass>
	void SetValue(UBlackboardComponent* BlackboardComponent, FBlackboard::FKey KeyID, typename TDataClass::FDataType Value, typename TDataClass::FDataType& OldValue, bool& bWasSet,
		bool bApply, bool bForce = false)
	{
		if (bApply)
		{
			if (BlackboardKey.IsSet())
			{
				bWasSet = true;
				OldValue = BlackboardComponent->GetValue<TDataClass>(KeyID);
			}

			BlackboardComponent->SetValue<TDataClass>(KeyID, Value);
		}
		else
		{
			if (bWasSet || bForce)
			{
				BlackboardComponent->SetValue<TDataClass>(KeyID, OldValue);	
			}
			else
			{
				BlackboardComponent->ClearValue(KeyID);
			}
		}
	}
};
