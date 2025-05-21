#include "Subsystems/QuestNpcSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/QuestSystemLogChannels.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/QuestNPC.h"
#include "Interfaces/QuestSystemGameMode.h"

UQuestNpcSubsystem* UQuestNpcSubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject ? WorldContextObject->GetWorld()->GetSubsystem<UQuestNpcSubsystem>() : nullptr;
}

void UQuestNpcSubsystem::RegisterNpc(const FGameplayTag& NpcIdTag, AActor* Npc)
{
	if (!ensure(NpcIdTag.IsValid()))
		return;

	auto QuestNPC = Cast<IQuestNPC>(Npc);
	if (QuestNPC->IsQuestNpcUnique() && Npcs.Contains(NpcIdTag))
	{
		UE_VLOG(this, LogQuestSystem, Error, TEXT("Duplicate unique NPC is trying to be spawned: %s"), *NpcIdTag.ToString());
		ensure(false);
		return;
	}
	
	Npcs.Add(NpcIdTag, Npc);
}

void UQuestNpcSubsystem::UnregisterNpc(const FGameplayTag& NpcIdTag, AActor* Npc)
{
	Npcs.RemoveSingle(NpcIdTag, Npc);
}

TScriptInterface<IQuestNPC> UQuestNpcSubsystem::FindNpc(const FGameplayTag& NpcIdTag, const FVector& PlayerLocation)
{
	TScriptInterface<IQuestNPC> Npc = nullptr;
	TArray<TScriptInterface<IQuestNPC>> SuitableNpcs;
	Npcs.MultiFind(NpcIdTag, SuitableNpcs);
	if (SuitableNpcs.Num() > 0)
	{
		float MaxDistance = FLT_MAX;
		for (const auto SuitableNpc : SuitableNpcs)
		{
			float TestSqDistance = FVector::DistSquared(PlayerLocation, SuitableNpc->GetQuestNpcLocation());
			if (TestSqDistance < MaxDistance)
			{
				MaxDistance = TestSqDistance;
				Npc = SuitableNpc;
			}
		}
	}

	return Npc;
}

void UQuestNpcSubsystem::AddCustomAttitude(const FGameplayTagContainer& SourceCharacterIds,
                                           const FGameplayTagContainer& TargetCharacterIds, const FGameplayTag& NewAttitude, const float GameTimeLimit)
{
	auto QuestSystemGameMode = Cast<IQuestSystemGameMode>(GetWorld()->GetAuthGameMode());
	
	for (const auto& SourceCharacterId : SourceCharacterIds)
	{
		auto& CustomNpcAttitudes = CustomAttitudes.FindOrAdd(SourceCharacterId);
		FTimespan ValidUntilGameTime = GameTimeLimit > 0.f
			? QuestSystemGameMode->GetQuestSystemGameTime() + FTimespan::FromHours(GameTimeLimit)
			: FTimespan::Zero();
		CustomNpcAttitudes.Add(FCachedNpcAttitude{ TargetCharacterIds, NewAttitude, ValidUntilGameTime });
	}
}

void UQuestNpcSubsystem::SetCustomAttitudePreset(const FGameplayTagContainer& ForCharacterIds,
	const FGameplayTag& NewAttitude, float GameHoursDuration)
{
	TArray<TScriptInterface<IQuestNPC>> SuitableNPCs;
	for (const auto& CharacterId : ForCharacterIds)
	{
		Npcs.MultiFind(CharacterId, SuitableNPCs);
		for (const auto& SuitableNpc : SuitableNPCs)
			SuitableNpc->AddQuestAttitudePreset(NewAttitude, GameHoursDuration);
	}
}

const FGameplayTag& UQuestNpcSubsystem::GetForcedAttitude(const FGameplayTag& NpcIdTag, const FGameplayTagContainer& ActorTags, const FTimespan& GameTime)
{
	auto NpcAttitudes = CustomAttitudes.Find(NpcIdTag);
	if (!NpcAttitudes)
		return FGameplayTag::EmptyTag;

	for (int i = NpcAttitudes->Num() - 1; i >= 0; --i)
	{
		auto& NpcAttitude = (*NpcAttitudes)[i];
		if (NpcAttitude.TargetCharacters.HasAny(ActorTags))
		{
			if (!NpcAttitude.ValidUntilGameTime.IsZero() && NpcAttitude.ValidUntilGameTime < GameTime)
			{
				NpcAttitudes->RemoveAt(i);
				continue;
			}

			return NpcAttitude.Attitude;
		}
	}

	return FGameplayTag::EmptyTag;
}

TArray<TScriptInterface<IQuestNPC>> UQuestNpcSubsystem::GetNpcs(const FGameplayTag& NpcId, const FGameplayTagQuery* OptionalFilter) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UQuestNpcSubsystem::GetNpcs)
	
	TArray<TScriptInterface<IQuestNPC>> Result;
	Npcs.MultiFind(NpcId, Result);
	
	if (OptionalFilter != nullptr && !OptionalFilter->IsEmpty())
	{
		for (int i = Result.Num() - 1; i >= 0; --i)
		{
			auto NpcTags = Result[i]->GetQuestNpcOwnedTags();
			if (!OptionalFilter->Matches(NpcTags))
				Result.RemoveAt(i);
		}
	}
	
	return Result;
}

TArray<TScriptInterface<IQuestNPC>> UQuestNpcSubsystem::GetNpcsInRange(const FGameplayTag& NpcId, const FVector& QuerierLocation, float Range, const FGameplayTagQuery* OptionalFilter) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UQuestNpcSubsystem::GetNpcsInRange)
	
	TArray<TScriptInterface<IQuestNPC>> Result;
	Npcs.MultiFind(NpcId, Result);

	const float RangeSq = Range * Range;
	
	const bool bCheckFilter = OptionalFilter != nullptr && !OptionalFilter->IsEmpty();
	for (int i = Result.Num() - 1; i >= 0; --i)
	{
		if ((Result[i]->GetQuestNpcLocation() - QuerierLocation).SizeSquared() > RangeSq)
		{
			Result.RemoveAt(i);
		}
		else if (bCheckFilter)
		{
			auto NpcTags = Result[i]->GetQuestNpcOwnedTags();
			if (!OptionalFilter->Matches(NpcTags))
				Result.RemoveAt(i);
		}
	}
	
	return Result;
}

TScriptInterface<IQuestNPC> UQuestNpcSubsystem::FindAndTeleportExistingNpc(
	const FGameplayTag& NpcIdTag, const FVector& SpawnLocation, const FVector& PlayerLocation)
{
	if (auto Npc = FindNpc(NpcIdTag, PlayerLocation))
	{
		Npc->TeleportToQuestLocation(SpawnLocation);
		return Npc;
	}
	
	return nullptr;
}

void UQuestNpcSubsystem::ChangeTagsForNpcs(const FGameplayTag& NpcId, const FGameplayTagContainer& TagsToUpdate, bool bAdd, const FGameplayTagQuery* NpcFilter)
{
	TArray<TScriptInterface<IQuestNPC>> NpcsToUpdate;
	Npcs.MultiFind(NpcId, NpcsToUpdate);
	bool bNeedToCheckFilter = NpcFilter != nullptr && !NpcFilter->IsEmpty();
	for (const auto& NpcToUpdate : NpcsToUpdate)
	{
		if (bNeedToCheckFilter)
		{
			const FGameplayTagContainer& NpcTags = NpcToUpdate->GetQuestNpcOwnedTags();
			if (!NpcFilter->Matches(NpcTags))
				continue;
		}
		
		if (bAdd)
			NpcToUpdate->AddNpcQuestTags(TagsToUpdate);
		else
			NpcToUpdate->RemoveNpcQuestTags(TagsToUpdate);
	}
}

bool UQuestNpcSubsystem::TryRunQuestBehavior(const FQuestActionNpcRunBehavior& RunBehaviorData, const FGuid& QuestActionId, const FQuestSystemContext&
                                             QuestSystemContext)
{
	for (const auto& NpcId : RunBehaviorData.NpcIdsTags)
	{
		TArray<TScriptInterface<IQuestNPC>> RelevantNPCs;
		Npcs.MultiFind(NpcId, RelevantNPCs);
		if (RelevantNPCs.Num() <= 0)
			continue;

		for (const auto NPC : RelevantNPCs)
		{
			bool bCanActivateBehavior = true;
			if (!RunBehaviorData.RequiredTags.IsEmpty())
			{
				FGameplayTagContainer NpcTags = NPC->GetQuestNpcOwnedTags();
				bCanActivateBehavior = RunBehaviorData.RequiredTags.Matches(NpcTags);
			}
		
			if (bCanActivateBehavior)
			{
				RunQuestBehavior(NPC, RunBehaviorData.NpcQuestBehaviorDescriptor, QuestActionId, QuestSystemContext);
				if (RunBehaviorData.bFirstOnly)
					break;
			}
		}
	}
	
	return true;
}

bool UQuestNpcSubsystem::RunQuestBehavior(const TScriptInterface<IQuestNPC>& Npc, const FNpcQuestBehaviorDescriptor& BehaviorDescriptor, const FGuid&
                                          QuestActionId, const FQuestSystemContext& QuestSystemContext)
{
	FNpcQuestBehaviorEndConditionContainer EndConditionsContainer;

	EndConditionsContainer.bAny = BehaviorDescriptor.bAnyEndConditionIsEnough;
	for (const auto& EndCondition : BehaviorDescriptor.QuestBehaviorEndConditions)
		EndConditionsContainer.EndConditions.Add(EndCondition.Get<FNpcQuestBehaviorEndConditionBase>().MakeProxy(QuestActionId, Npc.GetInterface(), QuestSystemContext));

	if (const auto ActiveQuestBehaviorId = ActiveNpcQuestBehaviorIds.Find(Npc.GetObject()))
	{
		const auto* NpcQuestBehaviorEndConditions = QuestBehaviorEndConditions.Find(*ActiveQuestBehaviorId);
		for (auto* EndCondition : NpcQuestBehaviorEndConditions->EndConditions)
			EndCondition->Disable();
		
		Npc->StopQuestBehavior();
	}

	ActiveNpcQuestBehaviorIds.Add(Npc.GetObject(), QuestActionId);
	QuestBehaviorEndConditions.Add(QuestActionId, EndConditionsContainer);
	Npc->RunQuestBehavior(BehaviorDescriptor.RequestedBehaviorIdTag);
	return true;
}

void UQuestNpcSubsystem::OnQuestBehaviorConditionTriggered(TWeakInterfacePtr<IQuestNPC> Npc, const FGuid& BehaviorActionId)
{
	const auto* NpcQuestBehaviorEndConditions = QuestBehaviorEndConditions.Find(BehaviorActionId);
	if (!NpcQuestBehaviorEndConditions->bAny)
	{
		for (const auto* EndCondition : NpcQuestBehaviorEndConditions->EndConditions)
			if (!EndCondition->IsCompleted())
				return;
	}
	else
	{
		for (auto* EndCondition : NpcQuestBehaviorEndConditions->EndConditions)
			EndCondition->Disable();
	}

	Npc->StopQuestBehavior();
	QuestBehaviorEndConditions.Remove(BehaviorActionId);
	ActiveNpcQuestBehaviorIds.Remove(Npc.GetObject());
}
