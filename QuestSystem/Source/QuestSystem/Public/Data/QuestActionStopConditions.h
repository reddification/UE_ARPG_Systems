// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.h"
#include "Interfaces/DelayedQuestActionInterface.h"

#include "QuestActionStopConditions.generated.h"

class IQuestNPC;
struct FQuestSystemContext;
class UNpcQuestBehaviorEndConditionProxyBase;

USTRUCT()
struct FNpcQuestBehaviorEndConditionBase
{
	GENERATED_BODY()
	
public:
	virtual ~FNpcQuestBehaviorEndConditionBase() = default;
	UNpcQuestBehaviorEndConditionProxyBase* MakeProxy(const FGuid& InQuestActionId, IQuestNPC* InQuestNPC, const FQuestSystemContext& QuestSystemContext) const;
	virtual UNpcQuestBehaviorEndConditionProxyBase* MakeProxyInternal(UObject* OwnerObject) const { return nullptr;}

};

USTRUCT(BlueprintType, DisplayName = "Until world state")
struct FNpcQuestBehaviorEndCondition_UntilWorldState : public FNpcQuestBehaviorEndConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery UntilWorldState;

	virtual UNpcQuestBehaviorEndConditionProxyBase* MakeProxyInternal(UObject* OwnerObject) const override;
};

USTRUCT(BlueprintType, DisplayName = "Until character state")
struct FNpcQuestBehaviorEndCondition_UntilCharacterState : public FNpcQuestBehaviorEndConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery UntilCharacterState;

	virtual UNpcQuestBehaviorEndConditionProxyBase* MakeProxyInternal(UObject* OwnerObject) const override;
};

USTRUCT(BlueprintType, DisplayName = "Game time duration")
struct FNpcQuestBehaviorEndCondition_GameTimeDuration : public FNpcQuestBehaviorEndConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag UntilDayTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0.01f, ClampMin = 0.01f, EditCondition = "UntilDayTime.IsValid() == false"))
	float GameTimeDurationHours = 1.f;

	virtual UNpcQuestBehaviorEndConditionProxyBase* MakeProxyInternal(UObject* OwnerObject) const override;
};

USTRUCT(BlueprintType, DisplayName = "Until player died")
struct FNpcQuestBehaviorEndCondition_UntilPlayerDied : public FNpcQuestBehaviorEndConditionBase
{
	GENERATED_BODY()

	virtual UNpcQuestBehaviorEndConditionProxyBase* MakeProxyInternal(UObject* OwnerObject) const override;
};

UCLASS()
class UNpcQuestBehaviorEndConditionProxyBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(const FGuid& InQuestActionId, TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext);
	virtual void EndConditionTriggered(); 
	bool IsCompleted() const { return bEndConditionCompleted; }

	// Unsubscribe from whatever delegates are used to trigger end condition
	virtual void Disable() {  };
	
protected:
	bool bEndConditionCompleted = false;
	
	TWeakInterfacePtr<IQuestNPC> Npc;
	FQuestSystemContext QuestSystemContext;
	FGuid QuestActionId;
};

USTRUCT()
struct FNpcQuestBehaviorEndConditionContainer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UNpcQuestBehaviorEndConditionProxyBase*> EndConditions;

	UPROPERTY()
	bool bAny = false;

	UPROPERTY()
	FGameplayTag BehaviorTag;	
};

UCLASS()
class UNpcQuestBehaviorEndConditionProxy_UntilWorldState : public UNpcQuestBehaviorEndConditionProxyBase
{
	GENERATED_BODY()

	friend FNpcQuestBehaviorEndCondition_UntilWorldState;
	
public:
	virtual void Initialize(const FGuid& InQuestActionId, TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext) override;
	virtual void Disable() override;
	
protected:
	FGameplayTagQuery UntilWorldState;

private:
	void OnWorldStateChanged(const FGameplayTagContainer& NewWorldState);
};

// 16.12.2024 @AK: TODO support having any character to observe character state, not just the owner
UCLASS()
class UNpcQuestBehaviorEndConditionProxy_UntilCharacterState : public UNpcQuestBehaviorEndConditionProxyBase
{
	GENERATED_BODY()

	friend struct FNpcQuestBehaviorEndCondition_UntilCharacterState;
	
public:
	virtual void Initialize(const FGuid& InQuestActionId, TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext) override;
	virtual void Disable() override;
	
protected:
	
	FGameplayTagQuery UntilCharacterState;
	
private:
	void OnCharacterStateChanged(const IQuestNPC* QuestNPC, const FGameplayTagContainer& NewCharacterState);
};

UCLASS()
class UNpcQuestBehaviorEndConditionProxy_GameTimeDuration : public UNpcQuestBehaviorEndConditionProxyBase, public IDelayedQuestAction
{
	GENERATED_BODY()

	friend struct FNpcQuestBehaviorEndCondition_GameTimeDuration;
	
public:
	virtual void Initialize(const FGuid& InQuestActionId, TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext) override;
	virtual void Disable() override;
	virtual void StartDelayedAction(const FQuestSystemContext& QuestSystemContext) override;

	FGameplayTag UntilDayTime;
	float GameTimeDurationHours = 1.f;
};

UCLASS()
class UNpcQuestBehaviorEndConditionProxy_UntilPlayerDied : public UNpcQuestBehaviorEndConditionProxyBase, public IDelayedQuestAction
{
	GENERATED_BODY()

	friend struct FNpcQuestBehaviorEndCondition_UntilPlayerDied;
	
public:
	virtual void Initialize(const FGuid& InQuestActionId, TWeakInterfacePtr<IQuestNPC> InNpc, const FQuestSystemContext& InQuestSystemContext) override;
	virtual void Disable() override;
	virtual void StartDelayedAction(const FQuestSystemContext& InQuestSystemContext) override;
};

