#include "Components/NpcComponent.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "GameplayEffectExtension.h"
#include "Settings/NpcCombatSettings.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/AIGameplayTags.h"
#include "AbilitySet.h"
#include "ExtendableAbilitySystemComponentInterface.h"
#include "Data/LogChannels.h"
#include "Data/NpcBlackboardDataAsset.h"
#include "Data/NpcRealtimeDialoguesDataAsset.h"
#include "Data/NpcSettings.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcControllerInterface.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Subsystems/NpcRegistrationSubsystem.h"

UNpcComponent::UNpcComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcComponent::BeginPlay()
{
	Super::BeginPlay();

	if(bNpcComponentInitialized)
		return;
	
	bNpcComponentInitialized = true;
	
	OwnerNPC.SetObject(GetOwner());
	OwnerNPC.SetInterface(Cast<INpc>(GetOwner()));

	OwnerAliveCreature.SetObject(GetOwner());
	OwnerAliveCreature.SetInterface(Cast<INpcAliveCreature>(GetOwner()));

	OwnerPawn = Cast<APawn>(GetOwner());

	auto AIController = OwnerPawn->GetController();
	NpcController.SetObject(AIController);
	NpcController.SetInterface(Cast<INpcControllerInterface>(AIController));
	ensure(NpcController.GetInterface() != nullptr);
	
	auto AsTeamAgent = Cast<IGenericTeamAgentInterface>(GetOwner());
	AsTeamAgent->SetGenericTeamId(GetDefault<UNpcCombatSettings>()->DefaultPerceptionTeamId);

	if (const FNpcDTR* NpcDTR = NpcDTRH.GetRow<FNpcDTR>(""))
		InitializeNpcDTR(NpcDTR);

	RegisterDeathEvents();
	NpcAttitudesDurationGameTime = GetDefault<UNpcSettings>()->NpcAttitudesDurationGameTime;

	if (auto World = GetWorld())
		if (auto NRS = World->GetSubsystem<UNpcRegistrationSubsystem>())
			NRS->RegisterNpc(this);
}

void UNpcComponent::InitializeNpcDTR(const FNpcDTR* NpcDTR)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::InitializeNpcDTR)
	
	for (const UAbilitySet* AbilitySet : NpcDTR->AbilitySets)
	{
		OwnerNPC->GrantAbilitySet(AbilitySet);
	}

	BaseAttitudes = GetNpcDTR()->BaseAttitudes;
	OwnerNPC->GiveNpcTags(NpcDTR->DefaultTags);
	NpcGoalsParameters.Append(NpcDTR->NpcGoalAndReactionParameters);
}

void UNpcComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto World = GetWorld())
		if (auto NRS = World->GetSubsystem<UNpcRegistrationSubsystem>())
			NRS->UnregisterNpc(this);
	
	Super::EndPlay(EndPlayReason);
}

void UNpcComponent::AddTemporaryCharacterAttitude(const AActor* Character, const FGameplayTag& Attitude)
{
	// TODO
	// 1. Subscribe to IAliveCreature::OnDeathStarted to clean up the map
	if (const float* AttitudeDurationPtr = NpcAttitudesDurationGameTime.Find(Attitude); ensure(AttitudeDurationPtr))
	{
		auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
		const FTimespan& CurrentGameTime = NpcGameMode->GetARPGAIGameTime();
		FTemporaryCharacterAttitudeMemory& TemporaryCharacterAttitude = TemporaryCharacterAttitudes.FindOrAdd(Character);
		TemporaryCharacterAttitude.AttitudeTag = Attitude;
		TemporaryCharacterAttitude.ValidUntilGameTime = CurrentGameTime + FTimespan::FromHours(*AttitudeDurationPtr);
	}
}

void UNpcComponent::SetAttitudePreset(const FGameplayTag& InAttitudePreset)
{
	CurrentAttitudePreset = InAttitudePreset;
	if (CurrentTemporaryAttitudePreset.IsValid())
		return;
	
	SetAttitudePresetInternal(InAttitudePreset);
}

void UNpcComponent::SetTemporaryAttitudePreset(const FGameplayTag& InAttitudePreset)
{
	CurrentTemporaryAttitudePreset = InAttitudePreset;
	SetAttitudePresetInternal(InAttitudePreset);
}

void UNpcComponent::SetAttitudePresetInternal(const FGameplayTag& InAttitudePreset)
{
	auto NpcDTR = GetNpcDTR();
	if (!ensure(NpcDTR))
		return;
		
	if (InAttitudePreset.IsValid())
	{
		if (auto CustomAttitudesPtr = NpcDTR->CustomAttitudes.Find(InAttitudePreset); ensure(CustomAttitudesPtr))
			CustomAttitudes = *CustomAttitudesPtr;
	}

	BaseAttitudes = NpcDTR->BaseAttitudes;

	// TODO sort NpcAttributes priority
}

void UNpcComponent::ResetTemporaryAttitudePreset()
{
	CurrentTemporaryAttitudePreset = FGameplayTag::EmptyTag;
	SetAttitudePresetInternal(CurrentAttitudePreset);
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

const FGameplayTag& UNpcComponent::GetCurrentAttitudePreset() const
{
	return CurrentAttitudePreset;
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

void UNpcComponent::SetHostile(AActor* ToActor)
{
	auto CurrentAttitude = GetAttitude(ToActor);
	if (CurrentAttitude.MatchesTag(AIGameplayTags::AI_Attitude_Hostile))
		return;

	AddTemporaryCharacterAttitude(ToActor, AIGameplayTags::AI_Attitude_Hostile);
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
	return OwnerNPC != nullptr ? OwnerNPC->GetNpcIdTag() : FGameplayTag::EmptyTag;
}

const FNpcDTR* UNpcComponent::GetNpcDTR() const
{
	return NpcDTRH.GetRow<FNpcDTR>("");
}

// There are 3 sources of attitudes:
// 1. Game-forced attitude (by dialogue outcome/quest system)
// 2. Immediate temporal attitude (from threat/attack)
// 3. From active attitude preset

FGameplayTag UNpcComponent::GetAttitude(const AActor* Actor) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcComponent::GetAttitude);

	auto World = GetWorld();
	if (!IsValid(World)) // I assume this happens when world partition unloads NPC from game process memory
		return FGameplayTag::EmptyTag;
	
	auto NpcGameMode = Cast<INpcSystemGameMode>(World->GetAuthGameMode());
	const auto& GameTime = NpcGameMode->GetARPGAIGameTime();

	auto GameplayTagActor = Cast<IGameplayTagAssetInterface>(Actor);
	FGameplayTagContainer ActorTags;
	if (GameplayTagActor != nullptr)
	{
		GameplayTagActor->GetOwnedGameplayTags(ActorTags);
		const FGameplayTag& GameForcedAttitude = NpcGameMode->GetForcedAttitudeToActor(GetNpcIdTag(), ActorTags);
		if (GameForcedAttitude.IsValid())
			return GameForcedAttitude;
	}
	
	if (const auto* TemporaryAttitude = TemporaryCharacterAttitudes.Find(Actor))
	{
		if (TemporaryAttitude->ValidUntilGameTime > GameTime)
			return TemporaryAttitude->AttitudeTag;
		else
			TemporaryCharacterAttitudes.Remove(Actor);
	}
	
	for (const auto& NpcAttitude : CustomAttitudes.NpcAttitudes)
	{
		if (!NpcAttitude.CharacterTagsAndWorldState.IsEmpty() && NpcAttitude.CharacterTagsAndWorldState.Matches(ActorTags))
			return NpcAttitude.Attitude;
	}

	for (const auto& NpcAttitude : BaseAttitudes.NpcAttitudes)
	{
		if (!NpcAttitude.CharacterTagsAndWorldState.IsEmpty() && NpcAttitude.CharacterTagsAndWorldState.Matches(ActorTags))
			return NpcAttitude.Attitude;
	}

	return AIGameplayTags::AI_Attitude_Neutral;
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
#if WITH_EDITOR
	auto Pawn = Cast<APawn>(GetOwner());
	auto AIController = Pawn->GetController();
#endif
	
	if (bInActive && ActiveStateEffects.Contains(StateTag))
	{
#if WITH_EDITOR
		UE_VLOG(AIController, LogARPGAI_NpcStates, Verbose, TEXT("Not activating state %s because it is already active"), *StateTag.ToString());
#endif
		return;
	}
	else if (!bInActive && !ActiveStateEffects.Contains(StateTag))
	{
		#if WITH_EDITOR
			UE_VLOG(AIController, LogARPGAI_NpcStates, Verbose, TEXT("Not deactivating state %s because it is not active"), *StateTag.ToString());
		#endif	
	}
	
	UAbilitySystemComponent* AbilitySystemComponent = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ensure(AbilitySystemComponent))
		return;

	if (const FNpcDTR* MobDTR = GetNpcDTR())
	{
		if (const FGameplayEffectsWrapper* StateEffects = MobDTR->NpcStatesDataAsset->StateEffects.Find(StateTag))
		{
			if (!ensure(StateEffects->GameplayEffects.Num() > 0)) return;

			if (bInActive)
			{
				TArray<FActiveGameplayEffectHandle>& ActiveEffectHandles = ActiveStateEffects.FindOrAdd(StateTag);
				ActiveEffectHandles.Reset(StateEffects->GameplayEffects.Num());
			
				for (const auto& GameplayEffectClass : StateEffects->GameplayEffects)
				{
					if (!IsValid(GameplayEffectClass))
					{
#if WITH_EDITOR
						UE_VLOG(AIController, LogARPGAI_NpcStates, Warning, TEXT("Invalid gameplay effect class for state %s"), *StateTag.ToString());
#endif
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
				
#if WITH_EDITOR
				UE_VLOG(AIController, LogARPGAI_NpcStates, Verbose, TEXT("Activated NPC state %s"), *StateTag.ToString());
#endif
				
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

#if WITH_EDITOR
					UE_VLOG(AIController, LogARPGAI_NpcStates, Verbose, TEXT("Removed NPC state %s"), *StateTag.ToString());
#endif
					OnStateChanged.Broadcast(StateTag, false);
				}
				ActiveStateEffects.Remove(StateTag);
			}
		}
#if WITH_EDITOR
		else
		{
			ensure(false);
			UE_VLOG(AIController, LogARPGAI_NpcStates, Verbose, TEXT("Not found NPC state %s"), *StateTag.ToString());
		}
#endif
	}
}
