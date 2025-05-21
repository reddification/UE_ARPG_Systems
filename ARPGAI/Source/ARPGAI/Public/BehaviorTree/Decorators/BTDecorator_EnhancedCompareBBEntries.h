

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Decorators/BTDecorator_CompareBBEntries.h"
#include "BTDecorator_EnhancedCompareBBEntries.generated.h"

UENUM(BlueprintType)
enum class EEnhancedBlackBoardEntryComparison : uint8
{
	Equal,
	NotEqual,
	GreaterThan,
	LessThan,
	GreaterThanOrEqual,
	LessThanOrEqual
};

UCLASS()
class ARPGAI_API UBTDecorator_EnhancedCompareBBEntries : public UBTDecorator_CompareBBEntries
{
	GENERATED_BODY()
	
public:
	UBTDecorator_EnhancedCompareBBEntries();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEnhancedBlackBoardEntryComparison EnhancedBlackBoardEntryComparison;

private:
	template<class T>
	bool Compare(const T& A, const T& B) const
	{
		switch (EnhancedBlackBoardEntryComparison)
		{
			case EEnhancedBlackBoardEntryComparison::Equal:
				return A == B;
			case EEnhancedBlackBoardEntryComparison::NotEqual:
				return A != B;
			case EEnhancedBlackBoardEntryComparison::GreaterThan:
				return A > B;
			case EEnhancedBlackBoardEntryComparison::LessThan:
				return A < B;
			case EEnhancedBlackBoardEntryComparison::GreaterThanOrEqual:
				return A >= B;
			case EEnhancedBlackBoardEntryComparison::LessThanOrEqual:
				return A <= B;
				default:
					return false;
		}
	}
};
