

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UObject/Object.h"
#include "BTDecorator_Log.generated.h"

UCLASS(HideCategories=(FlowControl, Condition))
// UCLASS()
class ARPGAI_API UBTDecorator_Log : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_Log();

#if WITH_EDITORONLY_DATA 
	
public:
	virtual FString GetStaticDescription() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnNodeActivation(FBehaviorTreeSearchData& SearchData) override;
	virtual void OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString, FBlackboardKeySelector> LogParameters; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnActivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnDeactivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnActivate_Debug = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOnDeactivate_Debug = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAddOnScreenMessage = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bAddOnScreenMessage"), Category="Visual")
	FColor OnActivateMessageScreenColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bAddOnScreenMessage"), Category="Visual")
	FColor OnDeactivateMessageScreenColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bAddOnScreenMessage"), Category="Visual")
	float TimeToDisplayMessage = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bOnActivate"), Category="Message")
	FString ActivationMessage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bOnDeactivate"), Category="Message")
	FString DeactivationMessage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShowParamsInDescription = false;;

private:
	FString GetParameterStringValue(const UBlackboardComponent* BlackboardComponent, const FBlackboardKeySelector& BBKey) const;
	void ShowMessage(const UBehaviorTreeComponent& OwnerComp, const FString& Message, const FColor& Color) const;

#endif

};
