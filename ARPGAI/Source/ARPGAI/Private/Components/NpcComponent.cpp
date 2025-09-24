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
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
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

	if(bNpcComponentInitialized)
		return;
	
	bNpcComponentInitialized = true;
	auto OwnerPawnLocal = Cast<APawn>(GetOwner());
	OwnerPawn = OwnerPawnLocal;

	OwnerNPC.SetObject(OwnerPawnLocal);
	OwnerNPC.SetInterface(Cast<INpc>(OwnerPawnLocal));

	OwnerAliveCreature.SetObject(OwnerPawnLocal);
	OwnerAliveCreature.SetInterface(Cast<INpcAliveCreature>(OwnerPawnLocal));
	
	auto AIController = OwnerPawn->GetController();
	NpcController.SetObject(AIController);
	NpcController.SetInterface(Cast<INpcControllerInterface>(AIController));
	ensure(NpcController.GetInterface() != nullptr);
	
	auto AsTeamAgent = Cast<IGenericTeamAgentInterface>(OwnerPawnLocal);
	AsTeamAgent->SetGenericTeamId(GetDefault<UNpcCombatSettings>()->DefaultPerceptionTeamId);

	if (const FNpcDTR* NpcDTR = NpcDTRH.GetRow<FNpcDTR>(""))
		InitializeNpcDTR(NpcDTR);

	RegisterDeathEvents();

	if (auto World = GetWorld())
	{
		if (auto NRS = World->GetSubsystem<UNpcRegistrationSubsystem>())
			NRS->RegisterNpc(this);

		if (auto NSS = World->GetSubsystem<UNpcSquadSubsystem>())
		{
			NSS->RegisterNpc(GetNpcIdTag(), OwnerPawnLocal);
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
}

void UNpcComponent::InitializeNpcDTR(const FNpcDTR* NpcDTR)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::InitializeNpcDTR)
	
	for (const UAbilitySet* AbilitySet : NpcDTR->AbilitySets)
	{
		OwnerNPC->GrantAbilitySet(AbilitySet);
	}

	OwnerNPC->GiveNpcTags(NpcDTR->DefaultTags);
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
	for (const auto* NpcRealtimeDialogueDataAsset : NpcDTR->NpcPhrasesDataAssets)
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
	auto NpcDTR = GetNpcDTR();

	if (!NpcDTR->NpcBlackboardDataAsset->ConversationMoveToEqsBBKey.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsObject(NpcDTR->NpcBlackboardDataAsset->ConversationMoveToEqsBBKey.SelectedKeyName, const_cast<UEnvQuery*>(EnvQuery));
		return true;
	}

	return false;
}

bool UNpcComponent::ExecuteDialogueWalkRequest(const FVector& Location, float AcceptableRadius)
{
	auto Blackboard = GetBlackboardComponent();
	auto NpcDTR = GetNpcDTR();
	
	if (!NpcDTR->NpcBlackboardDataAsset->ConversationMoveToLocationBBKey.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsVector(NpcDTR->NpcBlackboardDataAsset->ConversationMoveToLocationBBKey.SelectedKeyName, Location);
		return true;
	}

	return false;
}

bool UNpcComponent::ExecuteDialogueWalkRequest(const AActor* ToCharacter, float AcceptableRadius)
{
	auto Blackboard = GetBlackboardComponent();
	auto NpcDTR = GetNpcDTR();

	if (!NpcDTR->NpcBlackboardDataAsset->ConversationGoToAcceptableRadiusBBKey.SelectedKeyName.IsNone())
		Blackboard->SetValueAsFloat(NpcDTR->NpcBlackboardDataAsset->ConversationGoToAcceptableRadiusBBKey.SelectedKeyName, AcceptableRadius);
	
	if (!NpcDTR->NpcBlackboardDataAsset->ConversationSecondaryActorBBKey.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsObject(NpcDTR->NpcBlackboardDataAsset->ConversationSecondaryActorBBKey.SelectedKeyName, const_cast<AActor*>(ToCharacter));
		return true;
	}

	return false;
}

void UNpcComponent::OnNpcEnteredDialogueWithPlayer(ACharacter* Character)
{
	auto NpcDTR = GetNpcDTR();
	if (!ensure(NpcDTR) || !ensure(NpcDTR->NpcBlackboardDataAsset) || !ensure(!NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName.IsNone()))
		return;

	auto Blackboard = GetBlackboardComponent();
	Blackboard->SetValueAsObject(NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName, Character);
	// setting bAcceptedConversation switches the execution branch
	Blackboard->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->bAcceptedConversationBBKey.SelectedKeyName, true);
}

void UNpcComponent::OnNpcExitedDialogueWithPlayer()
{
	auto AIController = Cast<AAIController>(OwnerPawn->GetController<AAIController>());
	FAIMessage AIMessage;
	AIMessage.Status = FAIMessage::Success;
	AIMessage.MessageName = AIGameplayTags::AI_BrainMessage_Dialogue_Player_Completed.GetTag().GetTagName();
	AIController->GetBrainComponent()->HandleMessage(AIMessage);

	auto NpcDTR = GetNpcDTR();
	if (!ensure(NpcDTR != nullptr) || !ensure(NpcDTR->NpcBlackboardDataAsset) || !ensure(!NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName.IsNone()))
		return;
	
	auto Blackboard = GetBlackboardComponent();
	Blackboard->ClearValue(NpcDTR->NpcBlackboardDataAsset->ConversationPartnerBBKey.SelectedKeyName);
	Blackboard->SetValueAsBool(NpcDTR->NpcBlackboardDataAsset->bAcceptedConversationBBKey.SelectedKeyName, false);
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

void UNpcComponent::OnDamageReceived(float DamageAmount, const FOnAttributeChangeData& ChangeData)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::OnDamageReceived)
	
	if (AActor* Mob = Cast<AActor>(OwnerNPC.GetObject()))
	{
		FGameplayEffectContextHandle GameplayEffectContext = ChangeData.GEModData->EffectSpec.GetContext();
		AActor* DamageSource = GameplayEffectContext.GetEffectCauser();
		FVector EventLocation = FVector::ZeroVector;
		FVector HitLocation = FVector::ZeroVector;
		if (DamageSource)
		{
			EventLocation = DamageSource->GetActorLocation();
		}
		
		if (const FHitResult* HitResult = GameplayEffectContext.GetHitResult())
		{
			HitLocation = HitResult->ImpactPoint;
			if (DamageSource == nullptr)
			{
				EventLocation = HitResult->TraceStart;
			}
		}
		else
		{
			HitLocation = Mob->GetActorLocation();
		}
		
		UAISense_Damage::ReportDamageEvent(GetWorld(), Mob, DamageSource, DamageAmount, EventLocation, HitLocation);
	}
}

const FGameplayTag& UNpcComponent::GetNpcIdTag() const
{
	return CachedNpcId;
}

const FNpcDTR* UNpcComponent::GetNpcDTR() const
{
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
	
	if (const FNpcDTR* NpcDTR = GetNpcDTR())
	{
		if (const FGameplayEffectsWrapper* StateEffects = NpcDTR->NpcStatesDataAsset->StateEffects.Find(StateTag))
		{
			if (!ensure(StateEffects->GameplayEffects_Obsolete.Num() > 0 || !StateEffects->ParametrizedGameplayEffects.IsEmpty())) return;

			if (StateEffects->ParametrizedGameplayEffects.IsEmpty() && StateEffects->GameplayEffects_Obsolete.Num() > 0)
			{
				ApplyGameplayEffectsForState_Obsolete(StateTag, SetByCallerParams, bInActive, StateEffects);
			}
			else if (!StateEffects->ParametrizedGameplayEffects.IsEmpty())
			{
				ApplyGameplayEffectsForState(StateTag, SetByCallerParams, bInActive, StateEffects);
			}
			else
			{
				ensure(false);
				UE_VLOG(OwnerPawn.Get(), LogARPGAI_NpcStates, Verbose, TEXT("Not found NPC state %s"), *StateTag.ToString());
			}
		}
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
