#include "Components/Controller/NpcPerceptionComponent.h"

#include "AIController.h"
#include "GameplayTagAssetInterface.h"
#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcAreasComponent.h"
#include "Components/NpcAttitudesComponent.h"
#include "Components/Controller/NpcMemoryComponent.h"
#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Data/NpcMemoryDataTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcActorTagsInterface.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/NpcPerceptionInterface.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Interfaces/NpcValueableItemInterface.h"
#include "Interfaces/NpcThreat.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Settings/NpcCombatSettings.h"
#include "Subsystems/NpcSquadSubsystem.h"

UNpcPerceptionComponent::UNpcPerceptionComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.bCanEverTick = true;
}

void UNpcPerceptionComponent::SetCombatLogicComponent(const UNpcCombatLogicComponent* InCombatMemoryProvider)
{
    CombatLogicComponent = InCombatMemoryProvider;
}

void UNpcPerceptionComponent::SetMemoryComponent(UNpcMemoryComponent* InMemoryComponent)
{
    MemoryComponent = InMemoryComponent;
}

void UNpcPerceptionComponent::SetPawn(APawn* InPawn)
{
    OwnerPawn = InPawn;
    NpcAttitudesComponent = GetNpcAttitudesComponent(InPawn);
    NpcAreasComponent = GetNpcAreasComponent(InPawn);
    if (auto NpcAliveInterface = Cast<INpcAliveCreature>(InPawn))
        NpcAliveInterface->OnNpcAliveCreatureDeathStarted.AddUObject(this, &UNpcPerceptionComponent::OnNpcOwnerDied);
    
    if (auto PawnNpcPerceptionInterface = Cast<INpcPerceptionInterface>(InPawn))
    {
        NpcPerceptionInterface.SetObject(InPawn);
        NpcPerceptionInterface.SetInterface(PawnNpcPerceptionInterface);
    }
 
    if (auto Npc = Cast<INpcActorTagsInterface>(InPawn))
    {
        Npc->OnTagsChangedEvent_NPC.AddUObject(this, &UNpcPerceptionComponent::OnNpcTagsChanged);
        OwnerNpcTags = Npc->GetTags_NPC();
    }
    
    NpcCombatSettings = GetDefault<UNpcCombatSettings>();
    
    GetWorld()->GetTimerManager().SetTimer(PerceptionCacheTimer, this, &UNpcPerceptionComponent::ProcessShortTermMemory,
     PerceptionCacheInterval, true);
  
}

void UNpcPerceptionComponent::BeginPlay()
{
    Super::BeginPlay();
    // OnTargetPerceptionUpdated.AddDynamic(this, &)
    OnTargetPerceptionForgotten.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionForgottenHandler);
    OnTargetPerceptionInfoUpdated.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionInfoUpdatedHandler);
    OnTargetPerceptionUpdated.AddDynamic(this, &UNpcPerceptionComponent::OnTargetPerceptionUpdatedHandler);
    if (auto Pawn = Cast<AAIController>(GetOwner())->GetPawn())
        SetPawn(Pawn);
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

void UNpcPerceptionComponent::ProcessShortTermMemory()
{
    FVector NpcLocation = OwnerPawn->GetActorLocation();
    UNpcSquadSubsystem* SquadSubsystem = UNpcSquadSubsystem::Get(this);
    auto Allies = SquadSubsystem->GetAllies(OwnerPawn.Get(), false);
    auto OwnerThreat = Cast<INpcThreat>(OwnerPawn.Get());
    MyDamageOutput = OwnerThreat->GetDamageOutput_NpcThreat();
    MyProtectionValue = OwnerThreat->GetAverageProtection_NpcThreat();
    if (NpcPerceptionInterface.GetObject() != nullptr)
       MyHearingLoudnessThreshold = NpcPerceptionInterface->GetHearingLoudnessThreshold_NPC();
    
    AccumulatedDamage = 0.f;
    ProcessShortTermMemory(NpcLocation, Allies);
    MergeAllyPerceptions(NpcLocation, Allies);
    for (const auto& Ally : Allies)
    {
        if (ShortTermCharacterMemory.Contains(Ally))
            continue;
        
        CacheVisualPerception(NpcLocation, Ally, 10.f, true, false);
        ShortTermCharacterMemory[Ally].DetectionSource = EDetectionSource::Assumption;
    }
    
    PostProcessSoundEvents(NpcLocation, Allies);
    
#if WITH_EDITOR
    for (const auto& STC : ShortTermCharacterMemory)
    {
        FString CharacterLogId = STC.Value.CharacterId.ToString();
        UE_VLOG(OwnerPawn.Get(), LogARPGAI_Perception, VeryVerbose, TEXT("%s accumulated short term received damage = %.2f"),
            *CharacterLogId, STC.Value.ShortTermAccumulatedDamage);
        UE_VLOG(OwnerPawn.Get(), LogARPGAI_Perception, VeryVerbose, TEXT("%s accumulated long term received damage = %.2f"),
            *CharacterLogId, STC.Value.LongTermAccumulatedReceivedDamage);
        UE_VLOG(OwnerPawn.Get(), LogARPGAI_Perception, VeryVerbose, TEXT("%s accumulated long term dealt damage = %.2f"),
            *CharacterLogId, STC.Value.LongTermAccumulatedDealtDamage);
    }
#endif
}

void UNpcPerceptionComponent::ProcessShortTermMemory(const FVector& NpcLocation, const TArray<APawn*>& Allies)
{
    ShortTermCharacterMemory.Empty();
    ShortTermSoundsMemory.Empty();
    ShortTermHazardsMemory.Empty();
    ShortTermValueablesMemory.Empty();
    
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
        auto CauserAliveCreature = Cast<INpcAliveCreature>(PerceivedActor);
        for (const auto& AIStimulus : DataIt.Value().LastSensedStimuli)
        {
            if (AIStimulus.IsExpired())
            {
                UE_VLOG(GetOwner(), LogARPGAI_Perception, VeryVerbose, TEXT("Stimulus of type %s is expired for %s"),
                     *AIStimulus.Type.Name.ToString(), *PerceivedActor->GetName());
                continue;
            }
            
            if (AIStimulus.Type == HearingSenseId && AIStimulus.Strength >= MyHearingLoudnessThreshold)
            {
                FGameplayTag SoundTag = FGameplayTag::RequestGameplayTag(AIStimulus.Tag, true);
                RememberHeardSound(NpcLocation, PerceivedActor, SoundTag, AIStimulus.StimulusLocation, 
                    Allies.Contains(PerceivedActor), AIStimulus.GetAge(), AIStimulus.Strength);
            }
            else
            {
                if (PerceivedActor->IsA<APawn>())
                {
                    if (AIStimulus.Type == SightSenseId)
                    {
                        const float ObservationTime = ActorsObservationTime.FindOrAdd(PerceivedActor);
                        CacheVisualPerception(NpcLocation, PerceivedActor, ObservationTime, Allies.Contains(PerceivedActor), AIStimulus.IsActive());
                    }
                    else if (AIStimulus.Type == DamageSenseId && CauserAliveCreature != nullptr)
                    {
                        CacheDamagePerception(PerceivedActor, AIStimulus.Strength);
                    }
                }        
                else if (auto ValueableItem = Cast<INpcValueableItemInterface>(PerceivedActor))
                {
                    if (AIStimulus.Type == SightSenseId)
                    {
                        FNpcValueableItemPerceptionData ItemData;
                        ItemData.ItemId = ValueableItem->GetItemTag_NPC();
                        ItemData.Distance = (PerceivedActor->GetActorLocation() - NpcLocation).Size();
                        ItemData.TimeSeen = ActorsObservationTime.FindOrAdd(PerceivedActor);
                        ItemData.Value = 1.f; // 8 Apr 2026: TODO evaluate items
                        if (auto ValueableItemTagsInterface = Cast<IGameplayTagAssetInterface>(PerceivedActor))
                            ValueableItemTagsInterface->GetOwnedGameplayTags(ItemData.ItemTags);
                        
                        ItemData.ItemTags.AddTag(ItemData.ItemId);
                        ShortTermValueablesMemory.Add(PerceivedActor, ItemData);
                    }
                }
            }
        }
    }
}

void UNpcPerceptionComponent::PostProcessSoundEvents(const FVector &NpcLocation, const TArray<APawn*>& Allies)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UNpcPerceptionComponent::PostProcessSoundEvents);
    for (auto& ActorSoundEvents : ShortTermSoundsMemory)
    {
        auto EventActor = ActorSoundEvents.Key.Get();
        for (auto& SoundEvent : ActorSoundEvents.Value)
        {
            const auto* CharacterEventMemory = ShortTermCharacterMemory.Find(EventActor);
            if (CharacterEventMemory != nullptr)
            {
                if (CharacterEventMemory->HasVisualDetection())
                    SoundEvent.Traits |= EPerceivedSoundTrait::CanSee;
                
                if (CharacterEventMemory->bAlly)
                    SoundEvent.Traits |= EPerceivedSoundTrait::ByAlly;
                
                if (CharacterEventMemory->DotProduct_OwnerFV_ToActor > 0.3f)
                    SoundEvent.Traits |= EPerceivedSoundTrait::InFront;
            }
            else
            {
                float DotProduct = (SoundEvent.Location - NpcLocation).GetSafeNormal2D() | OwnerPawn->GetActorForwardVector().GetSafeNormal2D();
                if (DotProduct > 0.3f)
                {
                    SoundEvent.Traits |= EPerceivedSoundTrait::InFront;
                    
                    FHitResult HitResult;
                    FCollisionQueryParams CollisionParams;
                    CollisionParams.AddIgnoredActor(OwnerPawn.Get());
                    CollisionParams.AddIgnoredActor(EventActor);
                    bool bCanSee = !GetWorld()->LineTraceSingleByChannel(HitResult, NpcLocation, SoundEvent.Location,
                                                                         ECC_Visibility, CollisionParams);
                    if (bCanSee)
                    {
                        SoundEvent.Traits |= EPerceivedSoundTrait::CanSee;
                        if (Allies.Contains(EventActor))
                            SoundEvent.Traits |= EPerceivedSoundTrait::ByAlly;
                    }
                }
            }
        }
    }
}

void UNpcPerceptionComponent::CacheVisualPerception(const FVector& NpcLocation, AActor* PerceivedActor,
                                                    float ObservationTime, bool bAlly, bool bActive)
{
    INpcAliveCreature* AliveCreature = Cast<INpcAliveCreature>(PerceivedActor);
    bool bNoVisualPerceptionYet = !ShortTermCharacterMemory.Contains(PerceivedActor);
    auto& CharacterPerception = ShortTermCharacterMemory.FindOrAdd(PerceivedActor);
    bNoVisualPerceptionYet |= (CharacterPerception.DetectionSource & EDetectionSource::VisualMemory) == EDetectionSource::None;
    if (AliveCreature)
        CharacterPerception.CharacterId = AliveCreature->GetTagId_NpcAliveCreature();
            
    CharacterPerception.Distance = (NpcLocation - PerceivedActor->GetActorLocation()).Size();
    FVector PerceivedActorFV = PerceivedActor->GetActorForwardVector();
    auto OwnerFV = OwnerPawn->GetActorForwardVector();
    CharacterPerception.ForwardVectorsDotProduct = OwnerFV | PerceivedActorFV;
    FVector Direction_FromTargetToNpc = (NpcLocation - PerceivedActor->GetActorLocation()).GetSafeNormal();
    CharacterPerception.DotProduct_ActorFV_ToOwner = Direction_FromTargetToNpc | PerceivedActorFV;
    CharacterPerception.DotProduct_OwnerFV_ToActor = OwnerFV | (-Direction_FromTargetToNpc);
    CharacterPerception.DetectionSource |= EDetectionSource::VisualMemory;
    if (bActive)
        CharacterPerception.DetectionSource |= EDetectionSource::VisualActive;
    
    bool bAlive = AliveCreature ? AliveCreature->IsAlive_NpcAliveCreature() : false;
    if (bAlive)
    {
        if (auto ThreatInterface = Cast<INpcThreat>(PerceivedActor))
        {
            CharacterPerception.DamageOutput = ThreatInterface->GetDamageOutput_NpcThreat();
            CharacterPerception.Protection = ThreatInterface->GetAverageProtection_NpcThreat();
            CharacterPerception.AttackRange = ThreatInterface->GetAttackRange_NpcThreat();
            CharacterPerception.bCharacterSeesNpc = ThreatInterface->CanSeeThreat(OwnerPawn.Get());
        }
    }
    
    const auto ThreatScoreDependency = NpcCombatSettings->ThreatDependency.GetRichCurveConst();
    const float EnemyCombatAdvantage = MyProtectionValue > 0.f ? CharacterPerception.DamageOutput / MyProtectionValue : 1.f;
    const float MyCombatAdvantage = CharacterPerception.Protection > 0.f ? MyDamageOutput / CharacterPerception.Protection : 1.f; 
    CharacterPerception.StaticThreatScore = ThreatScoreDependency->Eval(EnemyCombatAdvantage - MyCombatAdvantage);
            
    const float ActorMaxHealth = AliveCreature->GetMaxHealth_NpcAliveCreature();
    CharacterPerception.NormalizedHealth = bAlive ? AliveCreature->GetHealth_NpcAliveCreature() / ActorMaxHealth : 0.f;
    CharacterPerception.MaxHealth = ActorMaxHealth;
    CharacterPerception.bAlive = bAlive;
    if (NpcAttitudesComponent)
    {
        auto Attitude = NpcAttitudesComponent->GetAttitude(PerceivedActor);
        CharacterPerception.Attitude = Attitude;
        CharacterPerception.bHostile = Attitude.MatchesTag(AIGameplayTags::AI_Attitude_Hostile);
    }

    if (auto GameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(PerceivedActor))
        GameplayTagAssetInterface->GetOwnedGameplayTags(CharacterPerception.CharacterTags);
    
    if (CharacterPerception.CharacterId.IsValid())
        CharacterPerception.CharacterTags.AddTag(CharacterPerception.CharacterId);

    if (NpcAreasComponent && NpcAreasComponent->HasAreas())
    {
        const FGameplayTag ActorTerritorytateTag = NpcAreasComponent->IsLocationWithinNpcArea(
            PerceivedActor->GetActorLocation(), VisibleActorTerritoryBoundsCheckExtent)
            ? AIGameplayTags::Observation_Combat_Target_Area_Inside
            : AIGameplayTags::Observation_Combat_Target_Area_Outside;
        
        CharacterPerception.CharacterTags.AddTag(ActorTerritorytateTag);
    }
    
    if (AliveCreature)
    {
        FGuid ActorId = AliveCreature->GetId_NpcAliveCreature();
        if (auto LTM = MemoryComponent->LongTermMemory.Find(ActorId))
            CharacterPerception.CharacterTags.AppendTags(LTM->RememberedTraits.GetTraits());
    
        if (bActive)
            MemoryComponent->RememberLongTerm(PerceivedActor, CharacterPerception, ActorId);
    }
    
    CharacterPerception.TimeSeen = ObservationTime;
    CharacterPerception.bAlly = bAlly;
    
    if (CombatLogicComponent)
    {
        if (auto CombatMemoryData = CombatLogicComponent->GetActorCombatMemoryData(PerceivedActor))
        {
            CharacterPerception.LongTermAccumulatedReceivedDamage = CombatMemoryData->AccumulatedReceivedDamage;
            CharacterPerception.LongTermAccumulatedDealtDamage = CombatMemoryData->AccumulatedDealtDamage;
        }
    }
}

void UNpcPerceptionComponent::CacheDamagePerception(AActor* PerceivedActor, float ReceivedDamage)
{
    auto& CharacterPerception = ShortTermCharacterMemory.FindOrAdd(PerceivedActor);
    CharacterPerception.DetectionSource = CharacterPerception.DetectionSource | EDetectionSource::Damage;
    CharacterPerception.ShortTermAccumulatedDamage = ReceivedDamage;
    AccumulatedDamage += ReceivedDamage;
}

void UNpcPerceptionComponent::RememberHeardSound(const FVector& NpcLocation, AActor* PerceivedActor, const FGameplayTag& SoundTag,
    const FVector& SoundLocation, bool bByAlly, float SoundPerceptionAge, float Loudness)
{
    const float Distance = (NpcLocation - SoundLocation).Size();
    FHeardSoundMemory HeardSound;
    HeardSound.Location = SoundLocation;;
    HeardSound.SoundTag = SoundTag;
    HeardSound.Distance = Distance;
    HeardSound.Age = SoundPerceptionAge;
    HeardSound.Loudness = Loudness;
    
    if (bByAlly)
        HeardSound.Traits |= EPerceivedSoundTrait::ByAlly;
    
    if (NpcAreasComponent && NpcAreasComponent->HasAreas())
        if (!NpcAreasComponent->IsLocationWithinNpcArea(SoundLocation, SoundEventTerritoryBoundsCheckExtent))
            HeardSound.Traits |= EPerceivedSoundTrait::OutsideTerritory;
    
    auto& ActorSoundEvents = ShortTermSoundsMemory.FindOrAdd(PerceivedActor);
    ActorSoundEvents.Add(HeardSound);

    if (PerceivedActor->IsA<APawn>())
    {
        auto& CharacterPerception = ShortTermCharacterMemory.FindOrAdd(PerceivedActor);
        CharacterPerception.ProducedSounds.AddTag(SoundTag);
        CharacterPerception.DetectionSource = CharacterPerception.DetectionSource | EDetectionSource::Audio;
        CharacterPerception.DotProduct_OwnerFV_ToActor = OwnerPawn->GetActorForwardVector() | (SoundLocation - NpcLocation).GetSafeNormal();
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

        auto AllyController = Ally->GetController();
        if (IsValid(AllyController))
        {
            auto AllyPerceptionComponent = AllyController->FindComponentByClass<UNpcPerceptionComponent>();
            for (const auto& AllyCharacterPerception : AllyPerceptionComponent->ShortTermCharacterMemory)
            {
                bool bCanAdoptPerception = !ShortTermCharacterMemory.Contains(AllyCharacterPerception.Key.Get())
                    && AllyCharacterPerception.Value.IsHostile()
                    && AllyCharacterPerception.Value.IsAlive()
                    && AllyCharacterPerception.Key != OwnerPawn;
                // && AllyCharacterPerception.Value.DetectionSource & EDetectionSource::Ally == 0;
            
                if (bCanAdoptPerception)
                {
                    auto& CharacterPerception = ShortTermCharacterMemory.Add(AllyCharacterPerception.Key.Get(), AllyCharacterPerception.Value);
                    CharacterPerception.ShortTermAccumulatedDamage = 0.f;
                    CharacterPerception.Distance = (AllyCharacterPerception.Key->GetActorLocation() - NpcLocation).Size();
                    CharacterPerception.ProducedSounds = FGameplayTagContainer::EmptyContainer;
                    CharacterPerception.bCharacterSeesNpc = Cast<INpcThreat>(AllyCharacterPerception.Key.Get())->CanSeeThreat(OwnerPawn.Get());
                    CharacterPerception.DetectionSource = EDetectionSource::Ally;
                }
            }
        }
    }
}

void UNpcPerceptionComponent::AddExternalVisualPerception(AActor* TriggerActor, float ObservationTime)
{
    UNpcSquadSubsystem* SquadSubsystem = UNpcSquadSubsystem::Get(this);
    auto Allies = SquadSubsystem->GetAllies(OwnerPawn.Get(), true);
    CacheVisualPerception(OwnerPawn->GetActorLocation(), TriggerActor, ObservationTime, Allies.Contains(TriggerActor), true);
}

void UNpcPerceptionComponent::AddExternalAudioPerception(AActor* TriggerActor, const FGameplayTag& SoundTag, float Loudness, float MaxRange)
{
    UNpcSquadSubsystem* SquadSubsystem = UNpcSquadSubsystem::Get(this);
    auto Allies = SquadSubsystem->GetAllies(OwnerPawn.Get(), true);
    UAISense_Hearing::ReportNoiseEvent(this, TriggerActor->GetActorLocation(), Loudness, 
        TriggerActor, MaxRange, SoundTag.GetTagName());
    RememberHeardSound(OwnerPawn->GetActorLocation(), TriggerActor, SoundTag, TriggerActor->GetActorLocation(), 
        Allies.Contains(TriggerActor), 0.f, Loudness);
}

void UNpcPerceptionComponent::AddExternalDamagePerception(AActor* TriggerActor, float PerceivedDamage)
{
    UAISense_Damage::ReportDamageEvent(this, OwnerPawn.Get(), TriggerActor, PerceivedDamage, 
        TriggerActor->GetActorLocation(), OwnerPawn->GetActorLocation());
    CacheDamagePerception(TriggerActor, PerceivedDamage);
}

float UNpcPerceptionComponent::GetAccumulatedTimeSeen(AActor* Actor) const
{
    const auto* ObservationTime = ActorsObservationTime.Find(Actor);
    return ObservationTime ? *ObservationTime : 0.f;
}

float UNpcPerceptionComponent::GetAccumulatedDamage(bool bRecalculate) const
{
    if (bRecalculate)
    {
        auto DamageSenseId = UAISense::GetSenseID(UAISense_Damage::StaticClass());
        float CurrentAccumulatedDamage = 0.f;
        for (auto DataIt = GetPerceptualDataConstIterator(); DataIt; ++DataIt)
        {
            for (const auto& AIStimulus : DataIt.Value().LastSensedStimuli)
            {
                if (AIStimulus.IsExpired() || AIStimulus.Type != DamageSenseId)
                    continue;

                CurrentAccumulatedDamage += AIStimulus.Strength;
            }
        }

        return CurrentAccumulatedDamage;
    }
    else
    {
        return AccumulatedDamage;
    }
}

void UNpcPerceptionComponent::OnTargetPerceptionInfoUpdatedHandler(const FActorPerceptionUpdateInfo& UpdateInfo)
{
    if (!UpdateInfo.Target.IsValid())
        return;
    
    if (UpdateInfo.Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
    {
        bool bValid = UpdateInfo.Stimulus.IsValid();
        bool bExpired = UpdateInfo.Stimulus.IsExpired();
        if (bValid && UpdateInfo.Stimulus.Strength > 0.f)
        {
            if (!ActorsObservationTime.Contains(UpdateInfo.Target))
                ActorsObservationTime.Add(UpdateInfo.Target, 0.f);
        }
        else if (bExpired)
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
    if (Stimulus.Type == UAISense::GetSenseID(UAISense_Damage::StaticClass()))
    {
        bool bActive = Stimulus.IsActive();
        bool bExpired = Stimulus.IsExpired();
        ensure((bActive && bExpired) == false);
        if (bActive && !bExpired)
            NpcAttitudesComponent->OnHitReceivedFromActor(Actor); 
    }
    
    TargetPerceptionUpdatedNativeEvent.Broadcast(Actor, Stimulus);
}

void UNpcPerceptionComponent::OnNpcOwnerDied(AActor* Actor)
{
    if (OwnerPawn.IsValid())
    {
        auto AliveInterface = Cast<INpcAliveCreature>(OwnerPawn.Get());
        AliveInterface->OnNpcAliveCreatureDeathStarted.RemoveAll(this);
        if (auto World = GetWorld())
            World->GetTimerManager().ClearTimer(PerceptionCacheTimer);
    }
    
    OnTargetPerceptionInfoUpdated.RemoveAll(this);
    OnTargetPerceptionForgotten.RemoveAll(this);
    OnTargetPerceptionUpdated.RemoveAll(this);
}

void UNpcPerceptionComponent::OnNpcTagsChanged(AActor* Pawn, const FGameplayTagContainer& NewTags)
{
    if (ensure(Pawn == OwnerPawn))
        OwnerNpcTags = NewTags;
}

bool UNpcPerceptionComponent::CanMergePerception(APawn* Ally)
{
    if (!ShortTermCharacterMemory.Contains(Ally) || !ShortTermCharacterMemory[Ally].bAlive)
        return false;
    
    const float DistanceThreshold = (ShortTermCharacterMemory[Ally].DetectionSource & EDetectionSource::VisualMemory) != EDetectionSource::None 
        ? AllyPerceptionMergeDistanceThreshold_VisualContact
        : AllyPerceptionMergeDistanceThreshold_Assumption;
    
    return ShortTermCharacterMemory[Ally].Distance <= DistanceThreshold;; 
}