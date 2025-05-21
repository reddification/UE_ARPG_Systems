#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Composites/BTComposite_Utility.h"
#include "Data/CombatEvaluationData.h"
#include "Data/NpcDTR.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "BTService_ThreatEvaluator.generated.h"

class UNpcComponent;
class ALyraCharacter;
class UAIPerceptionComponent;
struct FBehaviorUtilityParameters;
using namespace NpcCombatEvaluation;

UCLASS(Category="Combat")
class ARPGAI_API UBTService_ThreatEvaluator : public UBTService
{
	GENERATED_BODY()

private:
	struct FBTCombatEvaluatorNodeMemory
	{
		float DamageScoreFactor = 1.f;
		float TeammateTargetScoreFactor = 1.f;
		float MobCoordinatorScoreFactor = 1.f;
		float UpdateInterval = 0.f;
		float MaxHealth = 0.f;
		TWeakObjectPtr<AActor> CurrentTarget;
		bool bIgnoreTeamDamage = true;
		bool bHadTarget = false;

		float BehaviorUtilities[MAX_UTILITY_NODES];
		FGameplayTag CurrentBehaviorTag = FGameplayTag::EmptyTag;
	};
	
public:
	UBTService_ThreatEvaluator();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector TargetBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector EvaluationIntervalBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutTargetThreatLevelBBKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FBlackboardKeySelector OutInvestigationLocationBBKey;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="AI.Behavior"))
	TMap<FGameplayTag, FBlackboardKeySelector> BehaviorUtilitiesBBKeys;
	
	// If difference between combat and retreat utility < threshold - don't switch utility
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f))
	float BehaviourUtilityDifferenceThreshold = 0.1f;

	// If difference between previous target and new target score < threshold - don't switch target 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f, UIMax = 1.f, ClampMax = 1.f))
	float BestTargetScoreDifferenceThreshold = 0.1f;

	// TODO move to combat parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -1.f, ClampMin = -1.f, UIMax = 1.f, ClampMax = 1.f))
	float ThreatToNpcDotProductIgnoreThreat = 0.15f;

	// TODO move to some asset, be that data table or global NPC perception config
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.f, ClampMin = 0.f))
	float TimeSeenToReact = 1.f;

private:
	void EvaluateThreats(UBehaviorTreeComponent& OwnerComp, FBTCombatEvaluatorNodeMemory* CombatEvaluatorNodeMemory);
	void ProcessDangerousItemPerception(FCombatEvaluationResult& OutResult, AActor* TargetActor, const FVector& MobLocation) const;
	void ProcessDangerousItemPerception(FCombatEvaluationResult& OutResult, AActor* Target, float DangerousItemScore, const FVector& MobLocation, EDetectionSource DetectionSource) const;
	bool IsHostile(const FCombatEvaluationParameters& Parameters, AActor* Actor, EDetectionSource DetectionSource) const;
	void ProcessPerception(const FCombatEvaluationParameters& Parameters, const FBTCombatEvaluatorNodeMemory* NodeMemory, FCombatEvaluationResult& OutResult) const;
	void ProcessCharacterVisualPerception(const FCombatEvaluationParameters& Parameters, FCombatEvaluationResult& OutResult,
	                                      ACharacter* PerceivedCharacter) const;
	void ProcessCharacterPerception(const FCombatEvaluationParameters& Parameters, const FBTCombatEvaluatorNodeMemory* NodeMemory, FCombatEvaluationResult& OutResult,
	                                float MobMaxHealth, const FAIStimulus& AIStimulus,
	                                ACharacter* PerceivedCharacter) const;
	void ReceiveTeammateAwareness(FCombatEvaluationResult& CombatEvaluationResult, const FCombatEvaluationParameters& CombatEvaluationParameters,
	                              const FBTCombatEvaluatorNodeMemory* CombatEvalulatorNodeMemory) const;
	bool AssignBestTarget(UNpcCombatLogicComponent* MobComponent, UBlackboardComponent* BlackboardComponent,
	                      FBTCombatEvaluatorNodeMemory* NodeMemory, FCombatEvaluationResult& CombatEvaluationResult, const FGameplayTag& BestBehaviorUtilityTag) const;
	void EvaluateBehaviorUtilities(const FBTCombatEvaluatorNodeMemory* CombatEvaluatorNodeMemory,
		const APawn* ThisNpc, const FNpcDTR* NpcDTR, FCombatEvaluationResult& CombatEvaluationResult, TMap<FGameplayTag, float>& BehaviorUtilitiesScores);
	float EvaluateBehaviorUtility(FCombatEvaluationResult& CombatEvaluationResult, const FBehaviorUtilityParameters& BehaviorUtilityParameters,
		const APawn* ThisNpc, const FBTCombatEvaluatorNodeMemory* NodeMemory, const FGameplayTag& BehaviorUtilityTag) const;
	void GetBestBehaviorUtility(const TMap<FGameplayTag, float>& BehaviorUtilities, FGameplayTag& OutBestBehaviorTypeTag,
		float& OutBestBehaviorUtility) const;
	void SetActiveThreats(UNpcCombatLogicComponent* MobComponent, const FCombatEvaluationResult& CombatEvaluationResult) const;
	FGameplayTag GetBestBehaviorTag(const TMap<FGameplayTag, float>& BehaviorUtilitiesScores) const;
};
