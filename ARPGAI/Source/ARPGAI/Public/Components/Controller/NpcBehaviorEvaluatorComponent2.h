#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NpcBehaviorEvaluatorComponent.h"
#include "BehaviorEvaluators/BehaviorEvaluator_Base.h"
#include "NpcBehaviorEvaluatorComponent2.generated.h"

class FBehaviorEvaluator_Base;
class UBehaviorTreeComponent;
class IBehaviorEvaluator;
struct FGameplayTag;

UENUM()
enum class EBehaviorEvaluatorStateRequestPriority : uint8
{
	Block,
	Latest
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcBehaviorEvaluatorComponent2 : public UNpcBehaviorEvaluatorComponent
{
	GENERATED_BODY()

private:
	struct FBehaviorEvaluatorWrapper
	{
		struct FStateRequest
		{
			FStateRequest(const FName& Id, const EBehaviorEvaluatorState RequestedState, double RequestedUntilWorldTime)
				: Id(Id), State(RequestedState), UntilWorldTime(RequestedUntilWorldTime) { }

			FStateRequest(const FName& Id, const EBehaviorEvaluatorState RequestedState) : Id(Id), State(RequestedState) { }

			FORCEINLINE bool IsTemporal() const { return UntilWorldTime.IsSet(); } 
			FORCEINLINE bool IsStale(double WorldTime) const { return UntilWorldTime.IsSet() && UntilWorldTime.GetValue() <= WorldTime; } 
			
			FName Id;
			EBehaviorEvaluatorState State;
			TOptional<double> UntilWorldTime;
		};
		
		TUniquePtr<FBehaviorEvaluator_Base> Evaluator;
		TWeakObjectPtr<const UBehaviorEvaluatorConfig_Base> Config;
		
		double LastUpdateAtWorldTime = 0;
		float NextUpdateInterval = 0.f;
		
		bool bCharacterStateComplies = false;

		EBehaviorEvaluatorState GetDesiredState(const double CurrentWorldTime, EBehaviorEvaluatorStateRequestPriority StateRequestPriority) const;
		bool IsRequestStateClean(const double WorldTime) const;
		bool IsCreated() const { return Evaluator.IsValid() && Config.IsValid(); }
		void Purge() { Evaluator.Reset(); Config.Reset(); }
		bool IsAccumulating() const;
		bool CanFreezeRegression() const { return IsCreated() && Config->bTickable; }
		
		void AddStateRequest(const FName& SourceId, EBehaviorEvaluatorState State);
		void AddStateRequest(const FName& SourceId, EBehaviorEvaluatorState State, double UntilWorldTime);
		void RemoveStateRequest(const FName& SourceId);
		uint8 UpdateTemporalRequests(double CurrentTime);
		
		FORCEINLINE uint8 GetBlockRequests(const double WorldTime) const { return GetValidRequestsCount(WorldTime, EBehaviorEvaluatorState::Blocked); };
		FORCEINLINE uint8 GetRelevancyRequests(const double WorldTime) const { return GetValidRequestsCount(WorldTime, EBehaviorEvaluatorState::Relevant); };

	protected:
		TArray<FStateRequest, TInlineAllocator<4>> StateRequests;
		
	private:
		void OnStateRequested(const FName& SourceId, EBehaviorEvaluatorState State);
		int32 GetValidRequestsCount(const double WorldTime, EBehaviorEvaluatorState RequestType) const;
	};

public:
	UNpcBehaviorEvaluatorComponent2();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RequestUpdateEvaluatorsStates();
	virtual void RequestEvaluatorsRelevant(const FGameplayTagContainer& EvaluatorTags, bool bActive, const FName& SourceId) override;
	virtual void RequestEvaluatorsBlocked(const FGameplayTagContainer& EvaluatorTags, bool bActive, const FName& SourceId) override;

	virtual void RequestEvaluatorRelevant(const FGameplayTag& EvaluatorTag, bool bActive, const FName& SourceId) override;
	virtual void RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, bool bActive, const FName& SourceId) override;

	virtual void RequestEvaluatorRelevant(const FGameplayTag& EvaluatorTag, float Duration, const FName& SourceId) override;
	virtual void RequestEvaluatorBlocked(const FGameplayTag& EvaluatorTag, float Duration, const FName& SourceId) override;

	void Initialize(UBehaviorTreeComponent* InBTComponent);
	virtual bool SetBehaviorEvaluatorCooldown(const FGameplayTag& EvaluatorTag, float Cooldown, const FName& SourceId) override;

	void AddEvaluator(const UBehaviorEvaluatorConfig_Base* BehaviorEvaluatorConfig);
	void RemoveEvaluator(const FGameplayTag& EvaluatorId);
	
	// Called by BTDecorator_ActivateBehavior when BT control flow enters this behavior, driven by utility AI nodes 
	void ActivateBehavior(const FGameplayTag& BehaviorTag);

	// Called by BTDecorator_ActivateBehavior when BT control flow leaves this behavior, driven by utility AI nodes 
	void DeactivateBehavior(const FGameplayTag& BehaviorTag, EBehaviorEvaluatorResult Result);
	
	bool SetMaxUtility(const FGameplayTag& BehaviorTag);
	bool DelayRegression(const FGameplayTag& BehaviorTag, float Delay, bool bAppendToExisting);
	bool RequestFreezeRegression(const FGameplayTag& EvaluatorId, bool bFrozen);
	
	void HandleMessage(const FGameplayTag& EvaluatorTag, const FGameplayTag& Message);
	void BroadcastMessage(const FGameplayTag& Message);

	void HandleInteractionStateChanged(AActor* InteractionActor, const FGameplayTagContainer& InteractionTags, bool bActive);

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EBehaviorEvaluatorStateRequestPriority StateRequestPriority = EBehaviorEvaluatorStateRequestPriority::Latest;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DefaultComponentTickInterval = 0.1f;
	
	bool bPendingUpdateEvaluatorStates = false;

private:
	void UpdateTemporalRequests();
	void UpdateComponentTick();
	void SetEvaluatorState(FBehaviorEvaluatorWrapper& EvaluatorWrapper, EBehaviorEvaluatorState NewState);
	void UpdateEvaluatorsStates();
	void OnNpcTagsChanged(AActor* Npc, const FGameplayTagContainer& NewTags);
	
	TWeakObjectPtr<UBehaviorTreeComponent> BTComponent;
	TWeakObjectPtr<APawn> Pawn;
	
	TMap<FGameplayTag, FBehaviorEvaluatorWrapper> BehaviorEvaluators;
	FGameplayTagContainer NpcTags;
	bool bUpdatingEvaluatorsStates = false;
};
