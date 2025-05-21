


#include "Components/NpcPerceptionComponent.h"

#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

UNpcPerceptionComponent::UNpcPerceptionComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.bCanEverTick = true;
}

void UNpcPerceptionComponent::BeginPlay()
{
    Super::BeginPlay();
    // OnTargetPerceptionUpdated.AddDynamic(this, &)
    OnTargetPerceptionInfoUpdated.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionInfoUpdatedHandler);
    OnTargetPerceptionForgotten.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionForgottenHandler);
}

void UNpcPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (ActorsMemory.Num() > 0)
    {
        TArray<AActor*> CurrentlySeenActors;
        GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlySeenActors);
        for (auto& ActorMemory : ActorsMemory)
        {
            if (ActorMemory.Key.IsValid() && CurrentlySeenActors.Contains(ActorMemory.Key.Get()))
            {
                ActorMemory.Value.TimeSeen += DeltaTime;
            }
        }
    }
}

float UNpcPerceptionComponent::GetAccumulatedTimeSeen(AActor* Actor) const
{
    const auto* ActorMemory = ActorsMemory.Find(Actor);
    return ActorMemory ? ActorMemory->TimeSeen : 0.f;
}

float UNpcPerceptionComponent::GetAccumulatedDamage() const
{
    auto DamageSenseId = UAISense::GetSenseID(UAISense_Damage::StaticClass());
    float AccumulatedDamage = 0.f;
    for (auto DataIt = GetPerceptualDataConstIterator(); DataIt; ++DataIt)
    {
        for (const auto& AIStimulus : DataIt.Value().LastSensedStimuli)
        {
            if (AIStimulus.IsExpired() || AIStimulus.Type != DamageSenseId)
                continue;

            AccumulatedDamage += AIStimulus.Strength;
        }
    }

    return AccumulatedDamage;
}

void UNpcPerceptionComponent::RefreshStimulus(FAIStimulus& StimulusStore, const FAIStimulus& NewStimulus)
{
    // Accumulating damage instead of overriding it
    if (NewStimulus.Type == UAISense::GetSenseID(UAISense_Damage::StaticClass()))
    {
        if (StimulusStore.Strength > 0.f && !StimulusStore.IsExpired())
        {
            float InitialStrength = StimulusStore.Strength;
            StimulusStore = NewStimulus;
            StimulusStore.Strength += InitialStrength;
            return;
        }
    }
   
    Super::RefreshStimulus(StimulusStore, NewStimulus);
}

void UNpcPerceptionComponent::OnTargetPerceptionInfoUpdatedHandler(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    if (UpdateInfo.Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
    {
        if (UpdateInfo.Stimulus.Strength > 0.f && UpdateInfo.Stimulus.WasSuccessfullySensed())
        {
            if (!ActorsMemory.Contains(UpdateInfo.Target))
                ActorsMemory.Add(UpdateInfo.Target, FPerceptionMemoryData());
            
            // auto& ActorMemory = ActorsMemory.FindOrAdd(UpdateInfo.Target);
            // ActorMemory.TimeSeen = 0.f;
        }
        else if (UpdateInfo.Stimulus.IsExpired())
        {
            ActorsMemory.Remove(UpdateInfo.Target);
        }
    }
}

void UNpcPerceptionComponent::OnTargetPerceptionForgottenHandler(AActor* Actor)
{
    ActorsMemory.Remove(Actor);
}
