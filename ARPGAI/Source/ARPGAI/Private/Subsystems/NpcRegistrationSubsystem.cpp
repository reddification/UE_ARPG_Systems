// 


#include "Subsystems/NpcRegistrationSubsystem.h"

#include "Components/NpcComponent.h"
#include "Interfaces/Npc.h"

UNpcRegistrationSubsystem* UNpcRegistrationSubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject->GetWorld()->GetSubsystem<UNpcRegistrationSubsystem>();
}

void UNpcRegistrationSubsystem::RegisterNpc(UNpcComponent* NpcComponent)
{
	Npcs.Add(NpcComponent->GetNpcIdTag(), NpcComponent);
}

void UNpcRegistrationSubsystem::UnregisterNpc(UNpcComponent* NpcComponent)
{
	Npcs.Remove(NpcComponent->GetNpcIdTag(), NpcComponent);
}

TArray<UNpcComponent*> UNpcRegistrationSubsystem::GetNpcsInRange(const FVector& Origin,
                                                                 float Range, const TArray<FGameplayTagQuery>& NpcsFilters)
{
	TArray<UNpcComponent*> Result;

	TArray<FGameplayTag> NpcsIds;
	Npcs.GetKeys(NpcsIds);
	for (const auto& NpcId : NpcsIds)
	{
		TArray<TWeakObjectPtr<UNpcComponent>> NpcComponentsForId;
		Npcs.MultiFind(NpcId, NpcComponentsForId);
		for (const auto& NpcComponent : NpcComponentsForId)
		{
			auto RangeSq = (Origin - NpcComponent->GetOwner()->GetActorLocation()).SizeSquared();
			if (RangeSq > Range * Range)
				continue;

			bool bNpcPassesFilters = true;
			if (!NpcsFilters.IsEmpty())
			{
				auto NpcInterface = Cast<INpc>(NpcComponent->GetOwner());
				if (!NpcInterface)
					continue;

				FGameplayTagContainer NpcTags = NpcInterface->GetNpcOwnerTags();
				for (const auto& NpcFilter : NpcsFilters)
				{
					if (!NpcFilter.IsEmpty() && !NpcFilter.Matches(NpcTags))
					{
						bNpcPassesFilters = false;
						break;
					}
				}
			}

			if (bNpcPassesFilters)
				Result.Add(NpcComponent.Get());
		}
	}
	
	return Result;
}

// TODO dude we really need some octree representation or something, perhaps check out how it's done for smart objects 
TArray<UNpcComponent*> UNpcRegistrationSubsystem::GetNpcsInRange(const FGameplayTag& NpcId, const FVector& QuerierLocation,
                                                                 float Range, int CountLimit, const FGameplayTagQuery* NpcsFilter)
{
	TArray<UNpcComponent*> Result;
	
	TArray<TWeakObjectPtr<UNpcComponent>> AllNpcs;
	Npcs.MultiFind(NpcId, AllNpcs);

	TArray<TTuple<UNpcComponent*, float>> NpcsInRange;
	const bool bCheckFilters = NpcsFilter != nullptr && !NpcsFilter->IsEmpty();
	const float RangeSqThreshold = Range*Range;
	
	for (const auto& Npc : AllNpcs)
	{
		const float DistSq = (Npc->GetOwner()->GetActorLocation() - QuerierLocation).SizeSquared();
		if (DistSq < RangeSqThreshold && (!bCheckFilters || NpcsFilter->Matches(Npc->GetNpcTags())))
			NpcsInRange.Add({ Npc.Get(), DistSq } );
	}

	if (CountLimit > 1 && NpcsInRange.Num() > 1)
	{
		NpcsInRange.Sort([](const TTuple<UNpcComponent*, float>& A, const TTuple<UNpcComponent*, float>& B){ return A.Value < B.Value; });
		ensure(NpcsInRange[1].Value >= NpcsInRange[0].Value);
	}
	
	const int ResultCount = CountLimit > 0 ? FMath::Min(CountLimit, NpcsInRange.Num()) : NpcsInRange.Num();
	for (int i = 0; i < ResultCount; i++)
		Result.Add(NpcsInRange[i].Key);
	
	return Result;
}

UNpcComponent* UNpcRegistrationSubsystem::GetClosestNpc(const FGameplayTag& NpcId, const FVector& QuerierLocation,
                                                 const FGameplayTagQuery* NpcsFilters)
{
	UNpcComponent* ClosestNpc = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	
	TArray<TWeakObjectPtr<UNpcComponent>> AllNpcs;
	Npcs.MultiFind(NpcId, AllNpcs);

	const bool bCheckFilters = NpcsFilters != nullptr && !NpcsFilters->IsEmpty();
	for (const auto& Npc : AllNpcs)
	{
		const float DistSq = (Npc->GetOwner()->GetActorLocation() - QuerierLocation).SizeSquared();
		if (DistSq < ClosestDistanceSq && (!bCheckFilters || NpcsFilters->Matches(Npc->GetNpcTags())))
		{
			ClosestNpc = Npc.Get();
			ClosestDistanceSq = DistSq;
		}
	}

	return ClosestNpc;
}
