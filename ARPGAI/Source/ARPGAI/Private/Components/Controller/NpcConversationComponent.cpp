#include "Components/Controller/NpcConversationComponent.h"

#include "BlackboardKeyType_GameplayTag.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "GameFramework/Character.h"
#include "Interfaces/NpcActorTagsInterface.h"
#include "Interfaces/NpcAliveActor.h"
#include "Interfaces/NpcInteractionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

using namespace NpcConversation;

UNpcConversationComponent::UNpcConversationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNpcConversationComponent::SetPawn(APawn* InPawn)
{
	OwnerPawn = InPawn;
	
	if (auto NpcInteractionInterface = Cast<INpcInteractionInterface>(InPawn))
	{
		OwnerNpcInteractionInterface.SetObject(InPawn);
		OwnerNpcInteractionInterface.SetInterface(NpcInteractionInterface);
	}
	
	if (auto AliveCreatureInterface = Cast<INpcAliveActor>(InPawn))
	{
		AliveCreatureInterface->OnNpcAliveActorDeathStarted.AddUObject(this, &UNpcConversationComponent::OnOwnerDeathStarted);
	}
}

bool UNpcConversationComponent::ReceiveConversationRequest(APawn* Requestor, const FRequestParams& Params, FGameplayTag& OutRefuseReason)
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Received conversation [%s] request from %s"),  
		*Params.ConversationId.ToString(), *Requestor->GetName());
	
	if (Conversation.IsSet())
	{
		if (!Params.bByPlayer && !IsNewCollocutorHasPriorityOverCurrent(Requestor, Conversation.PrimaryCollocutor.Get()))
		{
			OnConversationRefused(Requestor, Params, AIGameplayTags::AI_Interaction_Dialogue_Refuse_AlreadyInConversation);
			return false;
		}
		else
		{
			if (Conversation.Role == ERole::Requestor)
				AbortConversation();
			else 
				LeaveConversation();
		}
	}
	
	bool bCanConversate = CanConversate(Requestor, OutRefuseReason);
	if (!bCanConversate)
	{
		OnConversationRefused(Requestor, Params, OutRefuseReason);
		return false;
	}
	
	bool bMaintainInteraction = false;
	if (OwnerNpcInteractionInterface && OwnerNpcInteractionInterface->IsInteracting_NPC())
	{
		bMaintainInteraction = true;
		if (Params.bForceSuspendActivity)
		{
			OwnerNpcInteractionInterface->StopInteracting_NPC();
			bMaintainInteraction = false;
		}
		else
		{
			if (!CanMaintainInteraction())
			{
				OwnerNpcInteractionInterface->StopInteracting_NPC();
				bMaintainInteraction = false;
			}
			else
			{
				const float DotProduct = OwnerPawn->GetActorForwardVector() | (Requestor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
				float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
				if (Angle > AbortInteractionAngleThreshold)
				{
					OwnerNpcInteractionInterface->StopInteracting_NPC();
					bMaintainInteraction = false;
				}
			}
		}	
	}
	
	Conversation.Role = ERole::Acceptor;
	Conversation.PrimaryCollocutor = Requestor;
	Conversation.ConversationId = Params.ConversationId;
	Conversation.bRealtime = Params.bRealtime;
	Conversation.AllCollocutors.Add(Requestor);
	OnConversationAccepted(Requestor, Params);
	if (bMaintainInteraction)
		CommitToPendingConversation();
	
	return true;
}

FRequestResult UNpcConversationComponent::RequestConversation(APawn* Target, const FRequestParams& ConversationParams, bool bResume)
{
	FRequestResult Result {};
	if (Conversation.IsActive())
	{
		ensure(false);
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Error, TEXT("WTF attempt to request conversation when there's currently an active conversation"));
		return Result;
	}	
	
	auto TargetConversationComponent = GetNpcConversationComponent(Target);
	if (TargetConversationComponent == nullptr)
	{
		OnConversationRequestFailed(ConversationParams);
		return Result;
	}
	
	Conversation = {};
	Conversation.ConversationId = ConversationParams.ConversationId;
	Conversation.PrimaryCollocutor = Target;
	Conversation.Role = ERole::Requestor;
	Conversation.AllCollocutors.Add(Target);
	Conversation.bRealtime = ConversationParams.bRealtime;
	
	const bool bAccepted = TargetConversationComponent->ReceiveConversationRequest(OwnerPawn, ConversationParams, Result.RefuseReason);
	if (!bAccepted)
	{
		OnConversationRequestFailed(ConversationParams);
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Verbose, TEXT("%s refused conversation %s. "),  
			*Target->GetName(), *ConversationParams.ConversationId.ToString());
		return Result;
	}
	
	// at this point primary target accepted conversation
	
	if (!ConversationParams.SecondaryConversationParticipants.IsEmpty())
	{
		auto NpcSubsystem = UNpcRegistrationSubsystem::Get(this);
		const FVector& PawnOwnerLocation = OwnerPawn->GetActorLocation();
		for (const auto& SecondaryConversationPartner : ConversationParams.SecondaryConversationParticipants)
		{
			TArray<UNpcComponent*> SecondaryNpcs = NpcSubsystem->GetNpcsInRange(SecondaryConversationPartner.CharacterId, PawnOwnerLocation,
				SecondaryConversationPartner.SearchInRange, SecondaryConversationPartner.Count, &SecondaryConversationPartner.CharacterFilter);

			for (const auto& SecondaryNpcComponent : SecondaryNpcs)
			{
				auto SecondaryConversationPawn = Cast<APawn>(SecondaryNpcComponent->GetOwner());
				auto SecondaryConversationConponent = GetNpcConversationComponent(SecondaryConversationPawn);
				if (SecondaryConversationConponent == nullptr)
					continue;
				
				FGameplayTag SecondaryRefuseReason;
				bool bSecondaryAccepted = SecondaryConversationConponent->ReceiveConversationRequest(OwnerPawn, ConversationParams, SecondaryRefuseReason);
				if (bSecondaryAccepted)
					Conversation.AllCollocutors.Add(SecondaryConversationPawn);
			}
		}
	}

	if (ConversationParams.bIncludePlayer)
		if (auto PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
			Conversation.AllCollocutors.Add(PlayerCharacter);
	
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Verbose, TEXT("Attempting to start conversation: %s"), *ConversationParams.ConversationId.ToString());
	
	Result.bSuccess = StartConversation(ConversationParams, bResume);
	if (Result.bSuccess)
		OnConversationInitiated(ConversationParams);
	else
		AbortConversation();	
	
	return Result;
}

void UNpcConversationComponent::OnCollocutorLeft(APawn* LeftCollocutor)
{
	if (Conversation.IsActive() && IsValid(LeftCollocutor))
	{
		UE_VLOG(GetOwner(), LogARPGAI_Conversation, VeryVerbose, TEXT("Collocutor %s left active conversation"), *LeftCollocutor->GetName());
		if (Conversation.PrimaryCollocutor == LeftCollocutor)
			StopConversation();
		
		Conversation.AllCollocutors.Remove(LeftCollocutor);
	}
}

void UNpcConversationComponent::AbortConversation()
{
	if (!Conversation.IsActive())
		return;
	
	if (Conversation.Role != ERole::Requestor)
	{
		UE_VLOG(GetOwner(), LogARPGAI_Conversation, Warning, TEXT("Conversation abort requested but I'm not conversation requestor. Must use LeaveConversation instead"));
		return;
	}
	
	if (!CanAbortConversation())
	{
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Requested to abort conversation, but CanAbortConversation forbits it. Conversation [%s]. Collocutor = %s"), 
			*Conversation.ConversationId.ToString(), Conversation.PrimaryCollocutor.IsValid() ? *Conversation.PrimaryCollocutor->GetName() : TEXT("None"));
		return;
	}
	
	UE_VLOG(GetOwner(), LogARPGAI_Conversation, VeryVerbose, TEXT("Conversation aborted"));
	Conversation.bLeaving = true;
	for (const auto& Collocutor : Conversation.AllCollocutors)
	{
		auto ConversationComponent = GetNpcConversationComponent(Collocutor.Get());
		ConversationComponent->ReceiveConversationAborted();
	}
	
	OnConversationAborted();
}

void UNpcConversationComponent::StopConversation()
{
	if (!Conversation.IsActive())
	{
		UE_VLOG(GetOwner(), LogARPGAI_Conversation, Warning, TEXT("Conversation stop requested when there's no active conversation"));
		return;
	}
	
	if (Conversation.Role != ERole::Requestor)
	{
		UE_VLOG(GetOwner(), LogARPGAI_Conversation, Warning, TEXT("Can't stop conversation because I am not the requestor. For acceptor LeaveConversation must be called"));
		return;
	}
	
	UE_VLOG(GetOwner(), LogARPGAI_Conversation, VeryVerbose, TEXT("Conversation stopped"));
	for (const auto& Collocutor : Conversation.AllCollocutors)
	{
		if (Collocutor.IsValid())
		{
			auto ConversationComponent = GetNpcConversationComponent(Collocutor.Get());
			ConversationComponent->OnConversationFinished();
		}
	}
	
	OnConversationFinished(); 
}

bool UNpcConversationComponent::CommitToPendingConversation()
{
	if (!Conversation.IsActive())
		return false;

	if (!ensure(Conversation.Role == ERole::Acceptor))
	{
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Warning, TEXT("Attempt to join pending conversation when role != acceptor. Current role = %d"), (int32)Conversation.Role);
		return false;
	}

	if (Conversation.bCommitted)
		return true;
	
	OnConversationJoined();
	if (auto CollocutorConversationComponent = GetNpcConversationComponent(Conversation.PrimaryCollocutor.Get()))
		CollocutorConversationComponent->OnCollocutorJoinedConversation(OwnerPawn);
	
	Conversation.bCommitted = true;
	return true;
}

void UNpcConversationComponent::LeaveConversation()
{
	if (!Conversation.IsActive())
		return;

	if (!CanLeaveConversation())
	{
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Requested to leave conversation, but CanLeaveConversation forbits it. Conversation [%s]. Collocutor = %s"), 
			*Conversation.ConversationId.ToString(), Conversation.PrimaryCollocutor.IsValid() ? *Conversation.PrimaryCollocutor->GetName() : TEXT("None"));
		return;
	}
	
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Leaving conversation [%s]. Collocutor = %s"), 
		*Conversation.ConversationId.ToString(), 
		Conversation.PrimaryCollocutor.IsValid() ? *Conversation.PrimaryCollocutor->GetName() : TEXT("None"));

	if (Conversation.Role == ERole::Requestor)
	{
		AbortConversation();
		return;
	}
	
	Conversation.bLeaving = true;
	if (ensure(Conversation.PrimaryCollocutor.IsValid()))
	{
		auto CollocutorComponent = GetNpcConversationComponent(Conversation.PrimaryCollocutor.Get());
		CollocutorComponent->OnCollocutorLeft(OwnerPawn);
	}
	
	OnConversationLeft();
}

void UNpcConversationComponent::OnConversationInitiated(const FRequestParams& Params)
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Initiated conversation [%s]. Collocutor = %s"), 
		*Conversation.ConversationId.ToString(), 
		Conversation.PrimaryCollocutor.IsValid() ? *Conversation.PrimaryCollocutor->GetName() : TEXT("None"));

	Conversation.bCommitted = true;
	ConversationEvent.Broadcast(Conversation, EEvent::Initiate);
	OnConversationJoined();
}

void UNpcConversationComponent::OnConversationRequestFailed(const FRequestParams& Params)
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Conversation [%s] request failed"), *Params.ConversationId.ToString());
	FinalizeConversation();
}

void UNpcConversationComponent::OnConversationAccepted(APawn* Requestor, const FRequestParams& Params)
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Accepted conversation [%s] requested by %s"), *Params.ConversationId.ToString(),
		*Requestor->GetName());
	ConversationEvent.Broadcast(Conversation, EEvent::Accept);	
}

void UNpcConversationComponent::OnConversationJoined()
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Joined conversation [%s]. Collocutor = %s"), 
		*Conversation.ConversationId.ToString(), 
		Conversation.PrimaryCollocutor.IsValid() ? *Conversation.PrimaryCollocutor->GetName() : TEXT("None"));
}

void UNpcConversationComponent::FinalizeConversation()
{
	Conversation = {};
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Verbose, TEXT("Conversation finalized"));
}

void UNpcConversationComponent::OnConversationFinished()
{
	if (!Conversation.IsActive())
		return;	
	
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Conversation [%s] finished. Collocutor = %s"), 
		*Conversation.ConversationId.ToString(), 
		Conversation.PrimaryCollocutor.IsValid() ? *Conversation.PrimaryCollocutor->GetName() : TEXT("None"));
	
	Conversation.bLeaving = true;
	ConversationEvent.Broadcast(Conversation, EEvent::Stop);
	FinalizeConversation();
}

void UNpcConversationComponent::OnConversationRefused(AActor* Requestor, const FRequestParams& Params,
	const FGameplayTag& RefuseReason)
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("Refused conversation [%s] requested by %s. Reason = %s"),  
		*Params.ConversationId.ToString(), *Requestor->GetName(), *RefuseReason.ToString());
}

void UNpcConversationComponent::OnConversationAborted()
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("On conversation aborted"));
	ConversationEvent.Broadcast(Conversation, EEvent::Abort);
	FinalizeConversation();
}

void UNpcConversationComponent::OnConversationLeft()
{
	UE_VLOG(OwnerPawn, LogARPGAI_Conversation, VeryVerbose, TEXT("On conversation left"));
	ConversationEvent.Broadcast(Conversation, EEvent::Abort);
	FinalizeConversation();
}

void UNpcConversationComponent::OnCollocutorJoinedConversation(APawn* JoinedColloctor)
{
	if (ensure(Conversation.IsSet()))
	{
		UE_VLOG(GetOwner(), LogARPGAI_Conversation, VeryVerbose, TEXT("Collocutor %s joined conversation [%s]"), *JoinedColloctor->GetName(),
			*Conversation.ConversationId.ToString());
	}
}

bool UNpcConversationComponent::IsNewCollocutorHasPriorityOverCurrent(AActor* NewRequestor, AActor* CurrentCollocutor)
{
	// 20 May 2026 (aki): this can be overriden for prioritizing player over NPCs
	return false;
}

bool UNpcConversationComponent::CanMaintainInteraction() const
{
	FGameplayTagContainer NpcTags;
	if (auto OwnerTagsInterface = Cast<INpcActorTagsInterface>(OwnerPawn))
		NpcTags = OwnerTagsInterface->GetTags_NPC();

	bool bCanMaintainActivity = NpcTags.HasTag(AIGameplayTags::AI_Interaction_ConversationCompatible);
	return bCanMaintainActivity;
}

void UNpcConversationComponent::OnOwnerDeathStarted(AActor* Actor, const FNpcDeathEventData& DeathEventData)
{
	if (Conversation.IsActive())
	{
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Verbose, TEXT("Owner death started while in conversation"));
		LeaveConversation();
	}
}

void UNpcConversationComponent::ReceiveConversationAborted()
{
	if (Conversation.IsActive())
	{
		UE_VLOG(OwnerPawn, LogARPGAI_Conversation, Verbose, TEXT("Received conversation aborted message"));
		Conversation.bLeaving = true;
		OnConversationAborted();
	}
}
