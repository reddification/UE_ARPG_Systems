#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTService_AreaRubberband.generated.h"

class UNpcAreasComponent;

UCLASS()
class ARPGAI_API UBTService_AreaRubberband : public UBTService
{
	GENERATED_BODY()
	
private:
	struct FBTMemory_AreaRubberband
	{
		float OutOfAreaDuration = 0.f;
		TWeakObjectPtr<UNpcAreasComponent> NpcAreasComponent;;
	};
	
public:
	UBTService_AreaRubberband();
	
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMemory_AreaRubberband); };
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector OutRubberbandingActiveBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float ActivationDelay = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FValueOrBBKey_Float AreaExtent = 300.f;
};
