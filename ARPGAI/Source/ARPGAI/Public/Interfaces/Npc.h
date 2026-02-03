// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/Tasks/BTTask_TriggerArbitraryNpcEvent.h"
#include "UObject/Interface.h"
#include "Npc.generated.h"

class UBlackboardComponent;

class IThreat;
// This class does not need to be modified.
UINTERFACE()
class UNpc : public UInterface
{
	GENERATED_BODY()
};

// 10.12.2024 @AK: TODO split INpc into INpcCombat, INpcBehavior and INpc
// Currently this interface just has too much of exposed methods for different kinds of tasks which are not related to each other in any way
class ARPGAI_API INpc
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FNpcStateChangedEvent, const FGameplayTagContainer& NewState);
	
public:
	virtual const FGameplayTag& GetNpcIdTag() const = 0;
	virtual const FDataTableRowHandle& GetNpcDataTableRowHandle() const = 0;
	virtual UBlackboardComponent* GetBlackboard() const = 0;
	virtual FGameplayTagContainer GetNpcOwnerTags() const = 0;
	virtual void GiveNpcTags(const FGameplayTagContainer& NewTags) = 0;
	virtual void RemoveNpcTags(const FGameplayTagContainer& TagsToRemove) = 0;
	
	virtual FGameplayAttribute GetAttackRangeAttribute() const = 0;
	
	virtual float GetAttackRange() const = 0;
	virtual float GetMoveSpeed() const = 0;
	virtual float GetCurrentSpeed() const = 0;
	virtual const FText& GetNpcName() const = 0;
	virtual bool RequestWeaponReady(bool bSetReady) = 0;
	virtual void CancelWeaponReady(bool bSetReady) = 0;
	
	virtual bool Dodge(const FVector& DodgeLocation) = 0;
	virtual void CancelDodge() = 0;
	virtual bool IsDodgeActive() const = 0;
	
	virtual bool PerformNpcGesture(const FGameplayTag& GestureTag) = 0;
	virtual void StopNpcGesture() = 0;
	
	virtual bool Parry() = 0;
	virtual void CancelParry() = 0;

	virtual void StartAttack() = 0;
	virtual void RequestNextAttack() = 0;
	virtual void CancelAttack() = 0;
	
	virtual void ChargeIn(float VerticalImpulseStrength, float ForwardImpulseStrength, const FVector& ToLocation) = 0;
	virtual bool IsChargeInActive() const = 0;
	virtual void CancelChargeIn() = 0;

	virtual double GetAttackPhaseEndTime() const = 0;
	
	// TODO refactor. Kinda bullshit decision. I need the AGameCharacter (project character) to store granted abilities handles
	// But from a solid standalone plugin architecture this is kinda bullshit - delegate such a thing to the owner
	virtual void GrantAbilitySet(const class UAbilitySet* AbilitySet) = 0;
	virtual IThreat* GetActiveTarget() const = 0;

	virtual void GiveMoney(int Gold) = 0;
	
	virtual void InterpolateToLocation(const FVector& Location, float InterpolateToSlotLocationRate, const TArray<const AActor*>& IgnoredActors, bool bSweep) = 0;

	virtual void LookAt(const FVector& Location) = 0;
	virtual void LookAt(const FRotator& Rotation) = 0;
	virtual void CancelLookAt() = 0;

	virtual bool IsAtLocation(const FGameplayTag& LocationId) const = 0;

	virtual bool StartDialogueWithPlayer(const FGameplayTag& OptionalDialogueId, const TArray<AActor*>& SecondaryDialogueParticipants, bool bForce) = 0;
	virtual void StopDialogueWithPlayer() = 0;
	
	virtual bool StartConversation(const FGameplayTag& ConversationId, const TArray<AActor*>& ConversationParticipants, bool bResume) = 0;
	virtual void LeaveConversation() = 0;
	virtual void StopConversation() = 0;
	virtual bool CanConversate(const AActor* ConversationPartner, FGameplayTag& OutRefuseReason) const = 0;

	virtual bool TriggerArbitraryNpcEvent(const FGameplayTag& EventTag) = 0;
	
	virtual const AActor* GetCatchUpTarget() = 0;
	virtual bool CanAttack() const = 0;

	virtual void SetForcedMoveSpeed(const float NewForcedMoveSpeed) = 0;
	virtual void ResetForcedMoveSpeed() = 0;

	virtual void StartInteractingWithSmartObject(AActor* SmartObjectActor, const struct FSmartObjectClaimHandle& SmartObjectClaimHandle) = 0;
	virtual void StopInteractingWithSmartObject() = 0;
	virtual bool IsNpcInteractingWithSmartObject() const = 0;

	FNpcStateChangedEvent OnNpcTagsChangedEvent;
};
