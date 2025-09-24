#include "Subsystems/QuestNpcSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/QuestSystemLogChannels.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/QuestCharacter.h"
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
		FDateTime ValidUntilGameTime = GameTimeLimit > 0.f
			? QuestSystemGameMode->GetQuestSystemGameTime() + FTimespan::FromHours(GameTimeLimit)
			: FDateTime();
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

const FGameplayTag& UQuestNpcSubsystem::GetForcedAttitude(const FGameplayTag& NpcIdTag, const FGameplayTagContainer& ActorTags, const FDateTime& GameTime)
{
	auto NpcAttitudes = CustomAttitudes.Find(NpcIdTag);
	if (!NpcAttitudes)
		return FGameplayTag::EmptyTag;

	for (int i = NpcAttitudes->Num() - 1; i >= 0; --i)
	{
		auto& NpcAttitude = (*NpcAttitudes)[i];
		if (NpcAttitude.TargetCharacters.HasAny(ActorTags))
		{
			if (NpcAttitude.ValidUntilGameTime.GetTicks() != 0 && NpcAttitude.ValidUntilGameTime < GameTime)
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
	bool bSuccess = false;
	for (const auto& NpcId : RunBehaviorData.NpcIdsTags)
	{
		TArray<TScriptInterface<IQuestNPC>> RelevantNPCs;
		Npcs.MultiFind(NpcId, RelevantNPCs);
		if (RelevantNPCs.Num() <= 0)
			continue;

		if (RunBehaviorData.bFirstOnly && RunBehaviorData.bPickNpcClosestToPlayer)
		{
			float DistSq = FLT_MAX;
			TScriptInterface<IQuestNPC> ClosestNpc = nullptr;
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
					float TestSqDistance = FVector::DistSquared(QuestSystemContext.Player->GetCharacterLocation(), NPC->GetQuestNpcLocation());
					if (TestSqDistance < DistSq)
					{
						DistSq = TestSqDistance;
						ClosestNpc = NPC;
					}
				}
			}

			if (ClosestNpc != nullptr)
				bSuccess = RunQuestBehavior(ClosestNpc, RunBehaviorData.NpcQuestBehaviorDescriptor, QuestActionId, QuestSystemContext);
		}
		else
		{
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
					// pay attention: order of "||" arguments matter. if it is bSuccess || RunQuestBehavior then RunQuestBehavior won't be executed after bSuccess is set to true once
					bSuccess = RunQuestBehavior(NPC, RunBehaviorData.NpcQuestBehaviorDescriptor, QuestActionId, QuestSystemContext) || bSuccess;
					if (RunBehaviorData.bFirstOnly)
						break;
				}
			}
		}
	}
	
	return bSuccess;
}

bool UQuestNpcSubsystem::RunQuestBehavior(const TScriptInterface<IQuestNPC>& Npc, const FNpcQuestBehaviorDescriptor& BehaviorDescriptor, const FGuid&
                                          QuestActionId, const FQuestSystemContext& QuestSystemContext)
{
	FNpcQuestBehaviorEndConditionContainer EndConditionsContainer;

	EndConditionsContainer.bAny = BehaviorDescriptor.bAnyEndConditionIsEnough;
	EndConditionsContainer.BehaviorTag = BehaviorDescriptor.RequestedBehaviorIdTag;
	for (const auto& EndCondition : BehaviorDescriptor.QuestBehaviorEndConditions)
		EndConditionsContainer.EndConditions.Add(EndCondition.Get<FNpcQuestBehaviorEndConditionBase>().MakeProxy(QuestActionId, Npc.GetInterface(), QuestSystemContext));

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

	Npc->StopQuestBehavior(NpcQuestBehaviorEndConditions->BehaviorTag);
	QuestBehaviorEndConditions.Remove(BehaviorActionId);
}
