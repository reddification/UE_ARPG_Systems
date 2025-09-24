
#include "Components/Controller/NpcPerceptionComponent.h"

#include "AIController.h"
#include "Components/NpcAttitudesComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Threat.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Subsystems/NpcSquadSubsystem.h"

using namespace NpcCombatEvaluation;

UNpcPerceptionComponent::UNpcPerceptionComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.bCanEverTick = true;
    // PrimaryComponentTick.TickInterval = 0.2f;
}

void UNpcPerceptionComponent::SetPawn(APawn* InPawn)
{
    OwnerPawn = InPawn;
    NpcAttitudesComponent = InPawn->FindComponentByClass<UNpcAttitudesComponent>();
    auto NpcAliveInterface = Cast<INpcAliveCreature>(InPawn);
    if (NpcAliveInterface)
        NpcAliveInterface->OnDeathStarted.AddUObject(this, &UNpcPerceptionComponent::OnNpcOwnerDied);
}

void UNpcPerceptionComponent::BeginPlay()
{
    Super::BeginPlay();
    // OnTargetPerceptionUpdated.AddDynamic(this, &)
    OnTargetPerceptionInfoUpdated.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionInfoUpdatedHandler);
    OnTargetPerceptionForgotten.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionForgottenHandler);
    OnTargetPerceptionUpdated.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionUpdatedHandler);
    auto Pawn = Cast<AAIController>(GetOwner())->GetPawn();
    if (Pawn)
        SetPawn(Pawn);

    GetWorld()->GetTimerManager().SetTimer(PerceptionCacheTimer, this, &UNpcPerceptionComponent::CachePerception,
        PerceptionCacheInterval, true);
}

bool UNpcPerceptionComponent::ConditionallyStoreSuccessfulStimulus(FAIStimulus& StimulusStore,
    const FAIStimulus& NewStimulus)
{
    // Accumulating damage instead of overriding it
    if (NewStimulus.Type == UAISense::GetSenseID(UAISense_Damage::StaticClass()))
    {
        if (StimulusStore.Strength > 0.f && !StimulusStore.IsExpired())
        {
            float InitialStrength = StimulusStore.Strength;
            StimulusStore = NewStimulus;
            StimulusStore.Strength += InitialStrength;
            return true;
        }
    }
    
    return Super::ConditionallyStoreSuccessfulStimulus(StimulusStore, NewStimulus);
}

void UNpcPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (ActorsObservationTime.Num() > 0)
    {
        TArray<AActor*> CurrentlySeenActors;
        GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlySeenActors);
        for (auto& ActorMemory : ActorsObservationTime)
            if (ActorMemory.Key.IsValid() && CurrentlySeenActors.Contains(ActorMemory.Key.Get()))
                ActorMemory.Value += DeltaTime;
    }
}

void UNpcPerceptionComponent::CachePerception()
{
    FVector NpcLocation = OwnerPawn->GetActorLocation();
    UNpcSquadSubsystem* SquadSubsystem = UNpcSquadSubsystem::Get(this);
    auto Allies = SquadSubsystem->GetAllies(OwnerPawn.Get(), false, true);
    CachePerception(NpcLocation, Allies);
    MergeAllyPerceptions(NpcLocation, Allies);
}

void UNpcPerceptionComponent::CachePerception(const FVector& NpcLocation, const TArray<APawn*>& Allies)
{
    CharacterPerceptionCache.Empty();
    HeardSoundsCache.Empty();
    HazardsPerceptionCache.Empty();
    
    auto SightSenseId = UAISense::GetSenseID(UAISense_Sight::StaticClass());
    auto DamageSenseId = UAISense::GetSenseID(UAISense_Damage::StaticClass());
    auto HearingSenseId = UAISense::GetSenseID(UAISense_Hearing::StaticClass());
    
    for (auto DataIt = GetPerceptualDataConstIterator(); DataIt; ++DataIt)
    {
        auto PerceivedActor = DataIt->Value.Target.Get();
        if (!IsValid(PerceivedActor))
            continue;

        if (PerceivedActor == OwnerPawn.Get())
            continue;

        UE_VLOG(GetOwner(), LogARPGAI_Perception, VeryVerbose, TEXT("Processing perception for %s"), *PerceivedActor->GetName());
        auto AliveCreature = Cast<INpcAliveCreature>(PerceivedActor);
        for (const auto& AIStimulus : DataIt.Value().LastSensedStimuli)
        {
            if (AIStimulus.IsExpired())
            {
                UE_VLOG(GetOwner(), LogARPGAI_Perception, VeryVerbose, TEXT("Stimulus of type %s is expired for %s"),
                     *AIStimulus.Type.Name.ToString(), *PerceivedActor->GetName());
                continue;
            }

            if (AIStimulus.Type == SightSenseId)
            {
                if (AliveCreature)
                {
                    auto& CharacterPerception = CharacterPerceptionCache.FindOrAdd(PerceivedActor);
                    auto GameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(PerceivedActor);
                    CharacterPerception.NpcId = AliveCreature->GetNpcAliveCreatureId();
                
                    CharacterPerception.Distance = (NpcLocation - PerceivedActor->GetActorLocation()).Size();
                    FVector PerceivedActorFV = PerceivedActor->GetActorForwardVector();
                    CharacterPerception.ForwardVectorsDotProduct = OwnerPawn->GetActorForwardVector() | PerceivedActorFV;
                    CharacterPerception.DirectionToForwardVectorDotProduct = (PerceivedActor->GetActorLocation() - NpcLocation).GetSafeNormal() | PerceivedActorFV;
                    CharacterPerception.DetectionSource = static_cast<EDetectionSource>(CharacterPerception.DetectionSource | Visual);
                    if (AliveCreature->IsNpcActorAlive())
                    {
                        auto ThreatInterface = Cast<IThreat>(PerceivedActor);
                        if (ThreatInterface)
                        {
                            CharacterPerception.Strength = ThreatInterface->GetStrength() + ThreatInterface->GetAverageProtection();
                            CharacterPerception.bCharacterSeesNpc = ThreatInterface->CanSeeThreat(OwnerPawn.Get());
                        }
                    }
                
                    CharacterPerception.Health = AliveCreature->GetHealth();
                    if (NpcAttitudesComponent.IsValid())
                        CharacterPerception.Attitude = NpcAttitudesComponent->GetAttitude(PerceivedActor);
                
                    if (GameplayTagAssetInterface)
                        GameplayTagAssetInterface->GetOwnedGameplayTags(CharacterPerception.NpcTags);

                    CharacterPerception.TimeSeen = ActorsObservationTime.FindOrAdd(PerceivedActor);
                    CharacterPerception.bAlly = Allies.Contains(PerceivedActor);
                }
            }
            else if (AIStimulus.Type == DamageSenseId)
            {
                if (AliveCreature)
                {
                    auto& CharacterPerception = CharacterPerceptionCache.FindOrAdd(PerceivedActor);
                    CharacterPerception.DetectionSource = static_cast<EDetectionSource>(CharacterPerception.DetectionSource | EDetectionSource::Damage);
                }
            }
            else if (AIStimulus.Type == HearingSenseId)
            {
                FGameplayTag SoundTag = FGameplayTag::RequestGameplayTag(AIStimulus.Tag, true);
                const float Distance = (NpcLocation - AIStimulus.StimulusLocation).Size();
                FHeardSounds HeardSound { AIStimulus.StimulusLocation, Distance, SoundTag, AIStimulus.GetAge(), Allies.Contains(PerceivedActor) };
                HeardSoundsCache.Add(PerceivedActor, HeardSound);
                
                if (AliveCreature)
                {
                    auto& CharacterPerception = CharacterPerceptionCache.FindOrAdd(PerceivedActor);
                    if (CharacterPerception.DetectionSource & EDetectionSource::Visual)
                    {
                        CharacterPerception.ProducedSounds.AddTag(SoundTag);
                        CharacterPerception.DetectionSource = static_cast<EDetectionSource>(CharacterPerception.DetectionSource | EDetectionSource::Audio);
                    }
                }
            }
        }
    }
}

void UNpcPerceptionComponent::MergeAllyPerceptions(const FVector& NpcLocation, const TArray<APawn*>& Allies)
{
    auto OwnerAttitudesComponent = OwnerPawn->FindComponentByClass<UNpcAttitudesComponent>();
    for (const auto& Ally : Allies)
    {
        if (!CanMergePerception(Ally))
            continue;

        if (OwnerAttitudesComponent)
            if (auto AllyAttitudesComponent = Ally->FindComponentByClass<UNpcAttitudesComponent>())
                AllyAttitudesComponent->ShareAttitudes(OwnerAttitudesComponent);

        if (IsValid(Ally->GetController()))
        {
            auto AllyPerceptionComponent = Ally->GetController()->FindComponentByClass<UNpcPerceptionComponent>();
            for (const auto& AllyCharacterPerception : AllyPerceptionComponent->CharacterPerceptionCache)
            {
                bool bCanAdoptPerception = !CharacterPerceptionCache.Contains(AllyCharacterPerception.Key.Get())
                    && AllyCharacterPerception.Value.IsHostile()
                    && AllyCharacterPerception.Value.IsAlive();
                // && AllyCharacterPerception.Value.DetectionSource & EDetectionSource::Ally == 0;
            
                if (bCanAdoptPerception)
                {
                    auto& CharacterPerception = CharacterPerceptionCache.Add(AllyCharacterPerception.Key.Get(), AllyCharacterPerception.Value);
                    CharacterPerception.AccumulatedDamage = 0.f;
                    CharacterPerception.Distance = (AllyCharacterPerception.Key->GetActorLocation() - NpcLocation).Size();
                    CharacterPerception.ProducedSounds = FGameplayTagContainer::EmptyContainer;
                    CharacterPerception.bCharacterSeesNpc = Cast<IThreat>(AllyCharacterPerception.Key.Get())->CanSeeThreat(OwnerPawn.Get());
                    CharacterPerception.DetectionSource = static_cast<EDetectionSource>(CharacterPerception.DetectionSource | EDetectionSource::Ally);
                }
            }
        }
    }
}

float UNpcPerceptionComponent::GetAccumulatedTimeSeen(AActor* Actor) const
{
    const auto* ObservationTime = ActorsObservationTime.Find(Actor);
    return ObservationTime ? *ObservationTime : 0.f;
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

void UNpcPerceptionComponent::OnTargetPerceptionInfoUpdatedHandler(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    if (UpdateInfo.Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
    {
        if (UpdateInfo.Stimulus.Strength > 0.f && UpdateInfo.Stimulus.WasSuccessfullySensed())
        {
            if (!ActorsObservationTime.Contains(UpdateInfo.Target))
                ActorsObservationTime.Add(UpdateInfo.Target, 0.f);
            
            // auto& ActorMemory = ActorsMemory.FindOrAdd(UpdateInfo.Target);
            // ActorMemory.TimeSeen = 0.f;
        }
        else if (UpdateInfo.Stimulus.IsExpired())
        {
            ActorsObservationTime.Remove(UpdateInfo.Target);
        }
    }
}

void UNpcPerceptionComponent::OnTargetPerceptionForgottenHandler(AActor* Actor)
{
    ActorsObservationTime.Remove(Actor);
}

void UNpcPerceptionComponent::OnTargetPerceptionUpdatedHandler(AActor* Actor, FAIStimulus Stimulus)
{
    TargetPerceptionUpdatedNativeEvent.Broadcast(Actor, Stimulus);
}

void UNpcPerceptionComponent::OnNpcOwnerDied(AActor* Actor)
{
    if (OwnerPawn.IsValid())
    {
        auto AliveInterface = Cast<INpcAliveCreature>(OwnerPawn.Get());
        AliveInterface->OnDeathStarted.RemoveAll(this);
        if (auto World = GetWorld())
            World->GetTimerManager().ClearTimer(PerceptionCacheTimer);
    }
}

bool UNpcPerceptionComponent::CanMergePerception(APawn* Ally)
{
    return CharacterPerceptionCache.Contains(Ally)
        && (CharacterPerceptionCache[Ally].DetectionSource & EDetectionSource::Visual) != 0
        && CharacterPerceptionCache[Ally].Distance <= AllyPerceptionMergeDistanceThreshold;
}

bool FCharacterPerceptionData::IsHostile() const
{
    return Attitude.MatchesTag(AIGameplayTags::AI_Attitude_Hostile); 
}
