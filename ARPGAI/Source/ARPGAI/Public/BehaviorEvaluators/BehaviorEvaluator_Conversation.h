// 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorEvaluator_Base.h"
#include "Components/Controller/NpcConversationComponent.h"
#include "BehaviorEvaluator_Conversation.generated.h"

class UNpcConversationComponent;
/**
 * 
 */
UCLASS(DisplayName="Conversation")
class ARPGAI_API UBehaviorEvaluatorConfig_Conversation : public UBehaviorEvaluatorConfig_Base
{
	GENERATED_BODY()
	
public:
	UBehaviorEvaluatorConfig_Conversation();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FBlackboardKeySelector OutConversationPartnerBBKey;


	
	// when conversation request, this value is applied to an active behavior evaluator as a reduction when the angle check doesnt pass
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BreakInteractionEffect = 0.2f;
	
	virtual TUniquePtr<FBehaviorEvaluator_Base> CreateEvaluator(UBehaviorTreeComponent* BTComponent) const override;
};

class FBehaviorEvaluator_Conversation : public FBehaviorEvaluator_Base
{
	
private:
	using Super = FBehaviorEvaluator_Base;

public:
	FBehaviorEvaluator_Conversation(UBehaviorTreeComponent& OwnerComp, const UBehaviorEvaluatorConfig_Base* const Config);
	
	virtual void SetState(EBehaviorEvaluatorState NewState) override;

protected:
	virtual void OnActivated() override;
	virtual void Cleanup() override;
	
private:
	TWeakObjectPtr<const UBehaviorEvaluatorConfig_Conversation> ConversationConfig;
	TWeakObjectPtr<UNpcConversationComponent> ConversationComponent;
};
