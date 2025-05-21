

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BTService.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTService_ActivateAbility.generated.h"


class UBehaviorTree;
/**
 * 
 */
UCLASS()
class ARPGAI_API UBTService_ActivateAbility : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_ActivateAbility();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery TagFilterQuery;
	
	UPROPERTY(EditAnywhere, Category=GameplayTagsToPass, meta=(Categories="AI.Ability.Event"))
	FGameplayTag AbilityTriggerEventTag;

	virtual bool CanActivateAbility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
	bool TrySendAbilityGameplayEvent(AActor* TargetActor, const FGameplayTag& Tag, const FGameplayEventData& Payload) const;
	
	UAbilitySystemComponent* GetAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const;
	
};
