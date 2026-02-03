#include "Components/NpcComponent.h"

#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "Settings/NpcCombatSettings.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySet.h"
#include "ExtendableAbilitySystemComponentInterface.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Data/NpcRealtimeDialoguesDataAsset.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcControllerInterface.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Perception/AIPerceptionComponent.h"
#include "Subsystems/NpcRegistrationSubsystem.h"
#include "Subsystems/NpcSquadSubsystem.h"

UNpcComponent::UNpcComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UNpcComponent::BeginPlay()
{
	Super::BeginPlay();
	
	const auto NpcDTR = GetNpcDTR();
	for (const UAbilitySet* AbilitySet : NpcDTR->AbilitySets)
		OwnerNPC->GrantAbilitySet(AbilitySet);

	OwnerNPC->GiveNpcTags(NpcDTR->DefaultTags);
	
	auto AIController = OwnerPawn->GetController();
	NpcController.SetObject(AIController);
	NpcController.SetInterface(Cast<INpcControllerInterface>(AIController));
	ensure(NpcController.GetInterface() != nullptr);
	
	if (auto World = GetWorld())
	{
		if (auto NRS = World->GetSubsystem<UNpcRegistrationSubsystem>())
			NRS->RegisterNpc(this);

		if (auto NSS = World->GetSubsystem<UNpcSquadSubsystem>())
		{
			NSS->RegisterNpc(GetNpcIdTag(), OwnerPawn.Get());
			if (DesiredSquadId.IsValid())
			{
				World->GetTimerManager().SetTimerForNextTick([NSS, this]()
				{
					NSS->JoinSquad(OwnerPawn.Get(), DesiredSquadId);
					NSS->RequestLeaderRole(OwnerPawn.Get());
				});
			}
		}
	}
}

void UNpcComponent::InitializeComponent()
{
	Super::InitializeComponent();
	CachedNpcId = FGameplayTag::RequestGameplayTag(NpcDTRH.RowName, false);
	InitializeNpcComponent();
}

void UNpcComponent::InitializeNpcComponent()
{
	if(bNpcComponentInitialized)
		return;
	
	bNpcComponentInitialized = true;
	
	auto OwnerPawnLocal = Cast<APawn>(GetOwner());
	OwnerPawn = OwnerPawnLocal;

	OwnerNPC.SetObject(OwnerPawnLocal);
	OwnerNPC.SetInterface(Cast<INpc>(OwnerPawnLocal));

	OwnerAliveCreature.SetObject(OwnerPawnLocal);
	OwnerAliveCreature.SetInterface(Cast<INpcAliveCreature>(OwnerPawnLocal));
	
	auto AsTeamAgent = Cast<IGenericTeamAgentInterface>(OwnerPawnLocal);
	AsTeamAgent->SetGenericTeamId(GetDefault<UNpcCombatSettings>()->DefaultPerceptionTeamId);

	if (const FNpcDTR* NpcDTR = NpcDTRH.GetRow<FNpcDTR>(""))
		InitializeNpcDTR(NpcDTR);

	RegisterDeathEvents();
}

void UNpcComponent::SetDTRH(const FDataTableRowHandle& InNpcDTRH)
{
	ensure(!IsValid(NpcDTRH.DataTable) || NpcDTRH.RowName.IsNone());
	NpcDTRH = InNpcDTRH;
}

void UNpcComponent::InitializeNpcDTR(const FNpcDTR* NpcDTR)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::InitializeNpcDTR)
	
	NpcBlackboardKeys = NpcDTR->NpcBlackboardDataAsset;
	NpcStates = NpcDTR->NpcStatesDataAsset;
	NpcPhrases = NpcDTR->NpcPhrasesDataAssets;
	NpcGoalsParameters.Append(NpcDTR->NpcGoalAndReactionParameters);
}

void UNpcComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto World = GetWorld())
	{
		if (auto NRS = World->GetSubsystem<UNpcRegistrationSubsystem>())
			NRS->UnregisterNpc(this);

		if (auto NSS = World->GetSubsystem<UNpcSquadSubsystem>())
			NSS->UnregisterNpc(GetNpcIdTag(), OwnerPawn.Get());
	}
	
	Super::EndPlay(EndPlayReason);
}

FGameplayTagContainer UNpcComponent::GetNpcTags() const
{
	return OwnerNPC->GetNpcOwnerTags();
}

const FNpcRealtimeDialogueLine* UNpcComponent::GetDialogueLine(const FGameplayTag& LineTagId) const
{
	auto NpcDTR = GetNpcDTR();
	// TODO cache it instead of gathering all tags every time
	FGameplayTagContainer NpcTags = GetNpcTags();
	const auto& WorldState = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode())->GetWorldState();
	NpcTags.AppendTags(WorldState);
	for (const auto* NpcRealtimeDialogueDataAsset : NpcPhrases)
	{
		auto Line = NpcRealtimeDialogueDataAsset->NpcPhrases.Find(LineTagId);
		if (Line && Line->LineOptions.Num() > 0)
			return &Line->LineOptions[FMath::RandRange(0, Line->LineOptions.Num() - 1)];	
	}
	
	return nullptr;
}

bool UNpcComponent::ExecuteDialogueWalkRequest(const UEnvQuery* EnvQuery, float AcceptableRadius)
{
	auto Blackboard = GetBlackboardComponent();
	if (!NpcBlackboardKeys->ConversationMoveToEqsBBKey.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsObject(NpcBlackboardKeys->ConversationMoveToEqsBBKey.SelectedKeyName, const_cast<UEnvQuery*>(EnvQuery));
		return true;
	}

	return false;
}

bool UNpcComponent::ExecuteDialogueWalkRequest(const FVector& Location, float AcceptableRadius)
{
	auto Blackboard = GetBlackboardComponent();
	if (!NpcBlackboardKeys->ConversationMoveToLocationBBKey.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsVector(NpcBlackboardKeys->ConversationMoveToLocationBBKey.SelectedKeyName, Location);
		return true;
	}

	return false;
}

bool UNpcComponent::ExecuteDialogueWalkRequest(const AActor* ToCharacter, float AcceptableRadius)
{
	auto Blackboard = GetBlackboardComponent();

	if (!NpcBlackboardKeys->ConversationGoToAcceptableRadiusBBKey.SelectedKeyName.IsNone())
		Blackboard->SetValueAsFloat(NpcBlackboardKeys->ConversationGoToAcceptableRadiusBBKey.SelectedKeyName, AcceptableRadius);
	
	if (!NpcBlackboardKeys->ConversationSecondaryActorBBKey.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsObject(NpcBlackboardKeys->ConversationSecondaryActorBBKey.SelectedKeyName, const_cast<AActor*>(ToCharacter));
		return true;
	}

	return false;
}

bool UNpcComponent::ExecuteDialogueFollowRequest(AActor* Actor)
{
	return SetDialogueFollowRequestState(const_cast<AActor*>(Actor), true);
}

bool UNpcComponent::ExecuteDialogueStopFollowingRequest()
{
	return SetDialogueFollowRequestState(nullptr, false);
}

bool UNpcComponent::SetDialogueFollowRequestState(AActor* Actor, bool bActive)
{
	auto Blackboard = GetBlackboardComponent();
	bool bAllBlackboardkeysExist = !NpcBlackboardKeys->ConversationSecondaryActorBBKey.SelectedKeyName.IsNone() 
		&& !NpcBlackboardKeys->ConversationFollowCounterpartBBKey.SelectedKeyName.IsNone();
	
	if (ensure(bAllBlackboardkeysExist))
	{
		// assignments must go in this order
		Blackboard->SetValueAsObject(NpcBlackboardKeys->ConversationSecondaryActorBBKey.SelectedKeyName, Actor);
		Blackboard->SetValueAsBool(NpcBlackboardKeys->ConversationFollowCounterpartBBKey.SelectedKeyName, bActive);
		return true;
	}

	return false;
}

void UNpcComponent::OnNpcEnteredDialogueWithPlayer(ACharacter* Character)
{
	if (!ensure(NpcBlackboardKeys) || !ensure(!NpcBlackboardKeys->ConversationPartnerBBKey.SelectedKeyName.IsNone()))
		return;

	auto Blackboard = GetBlackboardComponent();
	Blackboard->SetValueAsObject(NpcBlackboardKeys->ConversationPartnerBBKey.SelectedKeyName, Character);
	// setting bAcceptedConversation switches the execution branch
	Blackboard->SetValueAsBool(NpcBlackboardKeys->bAcceptedConversationBBKey.SelectedKeyName, true);
}

void UNpcComponent::OnNpcExitedDialogueWithPlayer()
{
	auto AIController = Cast<AAIController>(OwnerPawn->GetController<AAIController>());
	FAIMessage AIMessage;
	AIMessage.Status = FAIMessage::Success;
	AIMessage.MessageName = AIGameplayTags::AI_BrainMessage_Dialogue_Player_Completed.GetTag().GetTagName();
	AIController->GetBrainComponent()->HandleMessage(AIMessage);

	if (!ensure(NpcBlackboardKeys) || !ensure(!NpcBlackboardKeys->ConversationPartnerBBKey.SelectedKeyName.IsNone()))
		return;
	
	auto Blackboard = GetBlackboardComponent();
	Blackboard->ClearValue(NpcBlackboardKeys->ConversationPartnerBBKey.SelectedKeyName);
	Blackboard->SetValueAsBool(NpcBlackboardKeys->bAcceptedConversationBBKey.SelectedKeyName, false);
}

void UNpcComponent::StoreTaggedLocation(const FGameplayTag& DataTag, const FVector& Vector)
{
	StoredLocations.FindOrAdd(DataTag) = Vector;
}

void UNpcComponent::StoreTaggedActor(const FGameplayTag& DataTag, AActor* Actor)
{
	StoredActors.FindOrAdd(DataTag) = Actor;
}

AActor* UNpcComponent::GetStoredActor(const FGameplayTag& DataTag, bool bConsumeAfterReading)
{
	auto Result = StoredActors.Contains(DataTag) ? StoredActors[DataTag].Get() : nullptr;
	if (bConsumeAfterReading)
		StoredActors.Remove(DataTag);

	return Result;
}

FVector UNpcComponent::GetStoredLocation(const FGameplayTag& DataTag, bool bConsumeAfterReading)
{
	auto Result = StoredLocations.Contains(DataTag) ? StoredLocations[DataTag] : FAISystem::InvalidLocation;
	if (bConsumeAfterReading)
		StoredLocations.Remove(DataTag);
	
	return Result;
}

UBlackboardComponent* UNpcComponent::GetBlackboardComponent() const
{
	if (OwnerPawn.IsValid())
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
		{
			return AIController->GetBlackboardComponent();
		}
	}

	return nullptr;
}

void UNpcComponent::OnNpcDeathStarted(AActor* OwningActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::OnNpcDeathStarted)
	
	bDead = true;
	if (IsValid(NpcController.GetObject()))
	{
		NpcController->SetEnabled(false);
		if (auto OwnerController = OwnerPawn->GetController())
			OwnerController->SetLifeSpan(3.f);
	}

	GetOwner()->SetActorTickEnabled(false);
}

void UNpcComponent::OnNpcDeathFinished(AActor* OwningActor)
{
	//EnableRagdoll(Cast<ACharacter>(GetOwner())); // already called from death ability
}

const FGameplayTag& UNpcComponent::GetNpcIdTag() const
{
	return CachedNpcId;
}

const FNpcDTR* UNpcComponent::GetNpcDTR() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::GetNpcDTR)
	return NpcDTRH.GetRow<FNpcDTR>("");
}

void UNpcComponent::EnableRagdoll(ACharacter* Mob) const
{
	USkeletalMeshComponent* MobMesh = Mob->GetMesh();
	MobMesh->SetCollisionProfileName("Ragdoll");
	MobMesh->SetSimulatePhysics(true);
}

void UNpcComponent::RegisterDeathEvents()
{
	if (ensure(IsValid(OwnerNPC.GetObject())))
	{
		OwnerAliveCreature->OnDeathStarted.AddUObject(this, &UNpcComponent::OnNpcDeathStarted);
		OwnerAliveCreature->OnDeathFinished.AddUObject(this, &UNpcComponent::OnNpcDeathFinished);
	}
}

void UNpcComponent::SetStateActive(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams, bool bInActive)
{
	if (bInActive && ActiveStateEffects.Contains(StateTag))
	{
		UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Not activating state %s because it is already active"), *StateTag.ToString());
		return;
	}
	else if (!bInActive && !ActiveStateEffects.Contains(StateTag))
	{
		UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Not deactivating state %s because it is not active"), *StateTag.ToString());
	}
	
	if (const FGameplayEffectsWrapper* StateEffects = NpcStates->StateEffects.Find(StateTag))
	{
		if (!ensure(!StateEffects->GameplayEffects_Obsolete.IsEmpty() || !StateEffects->ParametrizedGameplayEffects.IsEmpty()))
		{
			UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Warning, TEXT("NPC state %s has no gameplay effects to apply. Consider removing"), *StateTag.ToString());
			return;
		}
		
		if (StateEffects->ParametrizedGameplayEffects.IsEmpty() && StateEffects->GameplayEffects_Obsolete.Num() > 0)
			ApplyGameplayEffectsForState_Obsolete(StateTag, SetByCallerParams, bInActive, StateEffects);
		else if (!StateEffects->ParametrizedGameplayEffects.IsEmpty())
			ApplyGameplayEffectsForState(StateTag, SetByCallerParams, bInActive, StateEffects);
		else
			{ ensure(false); }
	}
}

void UNpcComponent::ApplyGameplayEffectsForState_Obsolete(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams, bool bInActive, const FGameplayEffectsWrapper* StateEffects)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ensure(AbilitySystemComponent))
		return;

	if (bInActive)
	{
		TArray<FActiveGameplayEffectHandle>& ActiveEffectHandles = ActiveStateEffects.FindOrAdd(StateTag);
		ActiveEffectHandles.Reset(StateEffects->GameplayEffects_Obsolete.Num());
			
		for (const auto& GameplayEffectClass : StateEffects->GameplayEffects_Obsolete)
		{
			if (!IsValid(GameplayEffectClass))
			{
				UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Warning, TEXT("Invalid gameplay effect class for state %s"), *StateTag.ToString());
				continue;
			}
					
			auto EffectContext = AbilitySystemComponent->MakeEffectContext();
			auto EffectCDO = GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
			ensure(EffectCDO->DurationPolicy != EGameplayEffectDurationType::Instant); // I believe instant effects aren't really states
			auto EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContext);
			EffectSpec.Data->SetByCallerTagMagnitudes.Append(SetByCallerParams);
			auto AppliedEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
			ActiveEffectHandles.Add(AppliedEffectHandle);
		}
				
		UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Activated NPC state %s"), *StateTag.ToString());
		OnStateChanged.Broadcast(StateTag, true);
	}
	else
	{
		if (TArray<FActiveGameplayEffectHandle>* ActiveEffectHandles = ActiveStateEffects.Find(StateTag))
		{
			for (FActiveGameplayEffectHandle& ActiveGE : *ActiveEffectHandles)
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveGE);
			}

			UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Removed NPC state %s"), *StateTag.ToString());
			OnStateChanged.Broadcast(StateTag, false);
		}
		ActiveStateEffects.Remove(StateTag);
	}
}

void UNpcComponent::ApplyGameplayEffectsForState(const FGameplayTag& StateTag, const TMap<FGameplayTag, float>& SetByCallerParams,
                                                 bool bInActive, const FGameplayEffectsWrapper* StateEffects)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ensure(AbilitySystemComponent))
		return;

	if (bInActive)
	{
		TArray<FActiveGameplayEffectHandle>& ActiveEffectHandles = ActiveStateEffects.FindOrAdd(StateTag);
		ActiveEffectHandles.Reset(StateEffects->GameplayEffects_Obsolete.Num());
			
		for (const auto& ParametrizedGameplayEffect : StateEffects->ParametrizedGameplayEffects)
		{
			if (!IsValid(ParametrizedGameplayEffect.GameplayEffect))
			{
				UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Warning, TEXT("Invalid gameplay effect class for state %s"), *StateTag.ToString());
				continue;
			}
					
			auto EffectContext = AbilitySystemComponent->MakeEffectContext();
			auto EffectCDO = ParametrizedGameplayEffect.GameplayEffect->GetDefaultObject<UGameplayEffect>();
			ensure(EffectCDO->DurationPolicy != EGameplayEffectDurationType::Instant); // I believe instant effects aren't really states
			auto EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(ParametrizedGameplayEffect.GameplayEffect, 1.f, EffectContext);
			EffectSpec.Data->SetByCallerTagMagnitudes.Append(ParametrizedGameplayEffect.SetByCallerParameters);
			EffectSpec.Data->SetByCallerTagMagnitudes.Append(SetByCallerParams);
			auto AppliedEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data);
			ActiveEffectHandles.Add(AppliedEffectHandle);
		}
				
		UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Activated NPC state %s"), *StateTag.ToString());
		OnStateChanged.Broadcast(StateTag, true);
	}
	else
	{
		if (TArray<FActiveGameplayEffectHandle>* ActiveEffectHandles = ActiveStateEffects.Find(StateTag))
		{
			for (FActiveGameplayEffectHandle& ActiveGE : *ActiveEffectHandles)
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveGE);
			}

			UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Removed NPC state %s"), *StateTag.ToString());
			OnStateChanged.Broadcast(StateTag, false);
		}
		ActiveStateEffects.Remove(StateTag);
	}
}
