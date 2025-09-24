#include "Components/NpcAttitudesComponent.h"

#include "GameplayTagAssetInterface.h"
#include "Data/AIGameplayTags.h"
#include "Data/NpcDTR.h"
#include "Data/NpcSettings.h"
#include "Gameframework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"

UNpcAttitudesComponent::UNpcAttitudesComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcAttitudesComponent::BeginPlay()
{
	Super::BeginPlay();
	NpcAttitudesDurationGameTime = GetDefault<UNpcSettings>()->NpcAttitudesDurationGameTime;
}

void UNpcAttitudesComponent::InitializeNpcAttitudes(const FGameplayTag& InNpcId, const FDataTableRowHandle& InNpcDTRH)
{
	NpcDTRH = InNpcDTRH;
	auto NpcDTR = GetNpcDTR();
	BaseAttitudes = NpcDTR->BaseAttitudes;
	NpcId = InNpcId;
}

void UNpcAttitudesComponent::AddTemporaryCharacterAttitude(const AActor* Character, const FGameplayTag& Attitude, bool bShareableWithAllies)
{
	// TODO
	// 1. Subscribe to IAliveCreature::OnDeathStarted to clean up the map
	if (const float* AttitudeDurationPtr = NpcAttitudesDurationGameTime.Find(Attitude); ensure(AttitudeDurationPtr))
	{
		auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
		const FDateTime& CurrentGameTime = NpcGameMode->GetARPGAIGameTime();
		FTemporaryCharacterAttitudeMemory& TemporaryCharacterAttitude = TemporaryCharacterAttitudes.FindOrAdd(Character);
		TemporaryCharacterAttitude.AttitudeTag = Attitude;
		TemporaryCharacterAttitude.ValidUntilGameTime = CurrentGameTime + FTimespan::FromHours(*AttitudeDurationPtr);
		TemporaryCharacterAttitude.bShareableWithAllies = bShareableWithAllies;
	}
}

void UNpcAttitudesComponent::SetAttitudePreset(const FGameplayTag& InAttitudePreset)
{
	CurrentAttitudePreset = InAttitudePreset;
	if (CurrentTemporaryAttitudePreset.IsValid())
		return;
	
	SetAttitudePresetInternal(InAttitudePreset);
}

void UNpcAttitudesComponent::ClearAttitudePreset()
{
	CurrentAttitudePreset = FGameplayTag::EmptyTag;
	CustomAttitudes.NpcAttitudes.Empty();
}

void UNpcAttitudesComponent::SetTemporaryAttitudePreset(const FGameplayTag& InAttitudePreset)
{
	CurrentTemporaryAttitudePreset = InAttitudePreset;
	SetAttitudePresetInternal(InAttitudePreset);
}

const FNpcDTR* UNpcAttitudesComponent::GetNpcDTR() const
{
	return NpcDTRH.GetRow<FNpcDTR>("");
}

void UNpcAttitudesComponent::SetAttitudePresetInternal(const FGameplayTag& InAttitudePreset)
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

void UNpcAttitudesComponent::ResetTemporaryAttitudePreset()
{
	CurrentTemporaryAttitudePreset = FGameplayTag::EmptyTag;
	SetAttitudePresetInternal(CurrentAttitudePreset);
}

const FGameplayTag& UNpcAttitudesComponent::GetCurrentAttitudePreset() const
{
	return CurrentAttitudePreset;
}

void UNpcAttitudesComponent::SetHostile(AActor* ToActor, bool bLethal, bool bShareableWithAllies)
{
	auto CurrentAttitude = GetAttitude(ToActor);
	if (CurrentAttitude.MatchesTag(AIGameplayTags::AI_Attitude_Hostile))
		return;

	const FGameplayTag AttitudeTag = bLethal ? AIGameplayTags::AI_Attitude_Hostile_Lethal : AIGameplayTags::AI_Attitude_Hostile_NonLethal;
	AddTemporaryCharacterAttitude(ToActor, AttitudeTag, bShareableWithAllies);
}

void UNpcAttitudesComponent::ShareAttitudes(UNpcAttitudesComponent* OtherNpcAttitudesComponent) const
{
	for (const auto& TemporaryCharacterAttitude : TemporaryCharacterAttitudes)
		if (TemporaryCharacterAttitude.Value.bShareableWithAllies && !OtherNpcAttitudesComponent->TemporaryCharacterAttitudes.Contains(TemporaryCharacterAttitude.Key))
			OtherNpcAttitudesComponent->TemporaryCharacterAttitudes.Add(TemporaryCharacterAttitude.Key, TemporaryCharacterAttitude.Value);
}

// There are 3 sources of attitudes:
// 1. Game-forced attitude (by dialogue outcome/quest system)
// 2. Immediate temporal attitude (from threat/attack)
// 3. From active attitude preset
FGameplayTag UNpcAttitudesComponent::GetAttitude(const AActor* Actor) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UNpcAttitudesComponent::GetAttitude);

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
		const FGameplayTag& GameForcedAttitude = NpcGameMode->GetForcedAttitudeToActor(NpcId, ActorTags);
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
