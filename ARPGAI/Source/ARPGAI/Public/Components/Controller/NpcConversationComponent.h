#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Data/NpcActivitiesDataTypes.h"
#include "NpcConversationComponent.generated.h"


class INpcInteractionInterface;
class INpcConversationInterface;

namespace NpcConversation
{
	enum class EEvent : uint8
	{
		Initiate,
		Accept,
		Abort,
		Stop
	};

	struct FRequestParams
	{
		FGameplayTag ConversationId; // optional
		TArray<FNpcConversationSecondaryParticipantData> SecondaryConversationParticipants; // optional
		
		bool bIncludePlayer = false;
		bool bRealtime = false;
		bool bForceSuspendActivity = false;
		bool bByPlayer = false;

		FRequestParams() = default;
		
		FRequestParams(const FRequestParams& Other)
			: ConversationId(Other.ConversationId),
			  SecondaryConversationParticipants(Other.SecondaryConversationParticipants),
			  bIncludePlayer(Other.bIncludePlayer),
			  bRealtime(Other.bRealtime),
			  bForceSuspendActivity(Other.bForceSuspendActivity),
			  bByPlayer(Other.bByPlayer)
		{
		}

		FRequestParams(FRequestParams&& Other) noexcept
			: ConversationId(std::move(Other.ConversationId)),
			  SecondaryConversationParticipants(std::move(Other.SecondaryConversationParticipants)),
			  bIncludePlayer(Other.bIncludePlayer),
			  bRealtime(Other.bRealtime),
			  bForceSuspendActivity(Other.bForceSuspendActivity),
			  bByPlayer(Other.bByPlayer)
		{
		}

		FRequestParams& operator=(const FRequestParams& Other)
		{
			if (this == &Other)
				return *this;
			
			ConversationId = Other.ConversationId;
			SecondaryConversationParticipants = Other.SecondaryConversationParticipants;
			bIncludePlayer = Other.bIncludePlayer;
			bRealtime = Other.bRealtime;
			bForceSuspendActivity = Other.bForceSuspendActivity;
			bByPlayer = Other.bByPlayer;
			return *this;
		}

		FRequestParams& operator=(FRequestParams&& Other) noexcept
		{
			if (this == &Other)
				return *this;
			
			ConversationId = std::move(Other.ConversationId);
			SecondaryConversationParticipants = std::move(Other.SecondaryConversationParticipants);
			bIncludePlayer = Other.bIncludePlayer;
			bRealtime = Other.bRealtime;
			bForceSuspendActivity = Other.bForceSuspendActivity;
			bByPlayer = Other.bByPlayer;
			return *this;
		}
	};

	enum class ERole : uint8
	{
		None,
		Requestor,
		Acceptor,
	};

	struct FRequestResult
	{
		bool bSuccess = false;
		FGameplayTag RefuseReason;
	};

	struct FConversation
	{
		FGameplayTag ConversationId = FGameplayTag::EmptyTag;
		TWeakObjectPtr<APawn> PrimaryCollocutor = nullptr; // also present in AllCollocutors
		TArray<TWeakObjectPtr<APawn>> AllCollocutors; // except requestor
		
		bool bRealtime = false;
		ERole Role = ERole::None;
		bool bLeaving = false; // set to true when finalization begins to allow parts of conversation FSM to understand when not to trigger redundant finalizations 
		bool bCommitted = false; // if received conversation while interacting - instantly committed. otherwise NPC must call UBTTask_AcceptConversation

		FORCEINLINE bool IsSet() const { return PrimaryCollocutor.IsValid(); }
		FORCEINLINE bool IsActive() const { return IsSet() && !bLeaving; }
		FORCEINLINE bool IsRequestor() const { return Role == ERole::Requestor; };
	};
}

UCLASS(Blueprintable, ClassGroup=(Custom))
class ARPGAI_API UNpcConversationComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcConversationEvent, const NpcConversation::FConversation& Conversation, NpcConversation::EEvent Event);
	DECLARE_MULTICAST_DELEGATE(FNpcAbortActivityForConversationEvent)

public:
	UNpcConversationComponent();

	virtual void SetPawn(APawn* InPawn);

	virtual bool ReceiveConversationRequest(APawn* Requestor, const NpcConversation::FRequestParams& Params, FGameplayTag& OutRefuseReason);
	virtual NpcConversation::FRequestResult RequestConversation(APawn* Target, const NpcConversation::FRequestParams& ConversationParams, bool bResume);

	virtual void OnCollocutorLeft(APawn* Pawn);
	const NpcConversation::FConversation& GetCurrentConversation() const { return Conversation; }

	void AbortConversation();
	void StopConversation();
	void LeaveConversation();
	bool CommitToPendingConversation();

	FNpcConversationEvent ConversationEvent;

	virtual bool CanConversate(AActor* Actor, FGameplayTag& OutRefuseReson) const { return true; };
	
protected:
	// max angle between NPC view direction and conversation partner that allows to maintain interaction (smart object)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AbortInteractionAngleThreshold = 80.f;

	virtual void OnConversationInitiated(const NpcConversation::FRequestParams& Params);
	virtual void OnConversationRequestFailed(const NpcConversation::FRequestParams& Params);
	virtual void OnConversationAccepted(APawn* Requestor, const NpcConversation::FRequestParams& Params);
	
	// Means owner is "all in" in the conversation. Typically - for player - dialogue UI appeared and control halted, for NPC - NPC went into conversation subtree
	// This might not get called at all but conversation can still be valid and ongoing
	virtual void OnConversationJoined();
	
	virtual void OnConversationFinished();
	virtual void OnConversationRefused(AActor* Requestor, const NpcConversation::FRequestParams& Params, const FGameplayTag& RefuseReason);
	virtual void OnConversationAborted();
	virtual void OnConversationLeft();
	virtual void OnCollocutorJoinedConversation(APawn* JoinedColloctor);
	
	virtual bool IsNewCollocutorHasPriorityOverCurrent(AActor* NewRequestor, AActor* CurrentCollocutor);
	virtual bool StartConversation(const NpcConversation::FRequestParams& RequestParams, bool bResume) { return false; };
	virtual void FinalizeConversation();

	virtual bool CanMaintainInteraction() const;
	virtual bool CanLeaveConversation() const { return true; };
	virtual bool CanAbortConversation() const { return true; };
	
	virtual void OnOwnerDeathStarted(AActor* Actor);
	
	void ReceiveConversationAborted();
	
	NpcConversation::FConversation Conversation;
	
private:
	UPROPERTY()
	TScriptInterface<INpcInteractionInterface> OwnerNpcInteractionInterface = nullptr;
	
	UPROPERTY()
	APawn* OwnerPawn = nullptr;
};
