#include "Components/NpcAreasComponent.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Interfaces/NpcZone.h"
#include "Data/AiDataTypes.h"
#include "Data/AIGameplayTags.h"
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

void UNpcAreasComponent::AddAreaOfInterest(const FGameplayTag& AreaType, TScriptInterface<INpcZone> NewArea)
{
	FNpcAreasContainer& NpcAreasContainer = NpcAreas.FindOrAdd(AreaType);
	NpcAreasContainer.NpcAreas.Add(NewArea);
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
		}
	}
}

void UNpcAreasComponent::RemoveNpcAreas(const FGameplayTag& AreaType)
{
	NpcAreas.Remove(AreaType);
}

void UNpcAreasComponent::RemoveNpcArea(const FGameplayTag& AreaType, TScriptInterface<INpcZone> AreaToRemove)
{
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
	for (const auto& NpcAreaType : ActualNpcAreas)
		for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
			if (NpcArea->IsLocationWithinNpcArea(TestLocation, AreaExtent))
				return true;

	return false;
}
