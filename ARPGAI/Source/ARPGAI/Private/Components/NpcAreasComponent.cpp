#include "Components/NpcAreasComponent.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Interfaces/NpcZone.h"
#include "Data/AiDataTypes.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Gameframework/GameModeBase.h"

UNpcAreasComponent::UNpcAreasComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcAreasComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!AreasOfInterestTags.IsEmpty())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			AddAreasOfInterest(AIGameplayTags::Location_Arbitrary, AreasOfInterestTags);
		});
	}
}

void UNpcAreasComponent::AddAreaOfInterest(const FGameplayTag& AreaType, const TScriptInterface<INpcZone>& NewArea)
{
	FNpcAreasContainer& NpcAreasContainer = NpcAreas.FindOrAdd(AreaType);
	NpcAreasContainer.NpcAreas.Add(NewArea);
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("Added area of interest: %s [%s]"), *AreaType.ToString(), *NewArea->GetAreaId_NPC().ToString());
}

void UNpcAreasComponent::AddAreasOfInterest(const FGameplayTag& AreaType, const FGameplayTagContainer& AreaIds)
{
	auto NewAreasOfInterestArray = AreaIds.GetGameplayTagArray();
	auto GameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
	auto PawnOwner = Cast<APawn>(GetOwner());
	auto FoundNpcAreas = GameMode->GetNpcAreas(AreaIds, PawnOwner);
	auto& NpcAreaType = NpcAreas.FindOrAdd(AreaType);
	for (const auto& NpcAreaActor : FoundNpcAreas)
	{
		auto NpcAreaInterface = Cast<INpcZone>(NpcAreaActor);
		if (ensure(NpcAreaInterface))
		{
			TScriptInterface<INpcZone> NpcArea;
			NpcArea.SetObject(NpcAreaActor);
			NpcArea.SetInterface(NpcAreaInterface);
			NpcAreaType.NpcAreas.Add(NpcArea);
			UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("Added area of interest: %s [%s]"), *AreaType.ToString(), *NpcArea->GetAreaId_NPC().ToString());
		}
	}
}

void UNpcAreasComponent::RemoveNpcAreas(const FGameplayTag& AreaType)
{
	NpcAreas.Remove(AreaType);
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("Removed areas type: %s"), *AreaType.ToString());
}

void UNpcAreasComponent::RemoveNpcArea(const FGameplayTag& AreaType, const TScriptInterface<INpcZone>& AreaToRemove)
{
	UE_VLOG(GetOwner(), LogARPGAI_Activity, Verbose, TEXT("Removing NPC area: %s [%s]"), *AreaToRemove->GetAreaId_NPC().ToString(), *AreaType.ToString());
	if (auto NpcAreaContainer = NpcAreas.Find(AreaType))
		NpcAreaContainer->NpcAreas.Remove(AreaToRemove);
}

bool UNpcAreasComponent::SetExclusiveNpcAreaType(const FGameplayTag& NewExclusiveAreaType)
{
	if (!NpcAreas.Contains(NewExclusiveAreaType) || CurrentExclusiveAreaType.IsValid())
		return false;

	CurrentExclusiveAreaType = NewExclusiveAreaType;
	ExclusiveNpcAreas.Reset();
	ExclusiveNpcAreas.Add(NewExclusiveAreaType, NpcAreas[NewExclusiveAreaType]);
	return true;
}

bool UNpcAreasComponent::RemoveExclusiveNpcAreaType(const FGameplayTag& RemovedExclusiveAreaType)
{
	if (RemovedExclusiveAreaType != CurrentExclusiveAreaType)
		return false;
	
	CurrentExclusiveAreaType = FGameplayTag::EmptyTag;
	ExclusiveNpcAreas.Reset();
	return true;
}

const TMap<FGameplayTag, FNpcAreasContainer>& UNpcAreasComponent::GetNpcAreas() const
{
	return CurrentExclusiveAreaType.IsValid() ? ExclusiveNpcAreas : NpcAreas;
}

bool UNpcAreasComponent::IsLocationWithinNpcArea(const FVector& TestLocation, float AreaExtent) const
{
	const auto& ActualNpcAreas = GetNpcAreas();
	if (ActualNpcAreas.IsEmpty())
		return true;
	
	for (const auto& NpcAreaType : ActualNpcAreas)
		for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
			if (NpcArea->IsLocationWithinNpcArea(TestLocation, AreaExtent))
				return true;
	
	return false;
}

bool UNpcAreasComponent::HasAreas() const
{
	const auto& ActualNpcAreas = GetNpcAreas();
	return !ActualNpcAreas.IsEmpty();
}
