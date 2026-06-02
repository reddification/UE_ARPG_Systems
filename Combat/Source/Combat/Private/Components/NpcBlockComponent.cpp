#include "Components/NpcBlockComponent.h"

#include "AbilitySystemComponent.h"
#include "Data/CombatGameplayTags.h"
#include "Data/CombatLogChannels.h"
#include "Data/MeleeCombatSettings.h"
#include "GAS/Data/GameplayAbilityTargetData_ReceivedHit.h"
#include "Interfaces/ICombatant.h"
#include "Interfaces/NpcCombatant.h"

void UNpcBlockComponent::BeginPlay()
{
	Super::BeginPlay();
	NpcCombatant.SetObject(GetOwner());
	NpcCombatant.SetInterface(Cast<INpcCombatant>(GetOwner()));
	NpcHoldBlockDurationMin = GetDefault<UMeleeCombatSettings>()->AIHoldBlockDurationMin;
	NpcHoldBlockDurationMax = GetDefault<UMeleeCombatSettings>()->AIHoldBlockDurationMax;
	InitialBlockStrengthAccumulationScale = BlockStrengthAccumulationScale;
}

void UNpcBlockComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (auto WorldLocal = GetWorld())
	{
		auto& TimerManager = WorldLocal->GetTimerManager();
		TimerManager.ClearTimer(ReleaseBlockTimer);
		TimerManager.ClearTimer(StartBlockReactionDelayTimer);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UNpcBlockComponent::StartBlocking(const AActor* AttackingActor, EMeleeAttackType IncomingAttackType)
{
	PendingBlockInputs.Reset();
	bUsingExternalBlockInputSource = false;
	CurrentPendingBlockIndex = 0;

	float WeaponMastery = OwnerCombatant->GetActiveWeaponMasteryLevel();
	auto CombatSettings = GetDefault<UMeleeCombatSettings>();
	
	if (IncomingAttackType == EMeleeAttackType::Thrust)
	{
		PendingBlockInputs.Add(FVector2D(-0.4f, FMath::RandRange(-0.25f, 0.25f)).GetSafeNormal());
		PendingBlockInputs.Add(FVector2D(0.65f, FMath::RandRange(-0.25f, 0.25f)).GetSafeNormal());
	}
	else
	{
		FVector2D BestBlock = GetDesiredBlockVector(IncomingAttackType).GetSafeNormal();
		float Intelligence = NpcCombatant->GetIntelligence();
		float IntelligenceBlockErrorFactor = CombatSettings->IntellectToBlockErrorFactorDependency.GetRichCurveConst()->HasAnyData() 
			? CombatSettings->IntellectToBlockErrorFactorDependency.GetRichCurveConst()->Eval(Intelligence)
			: Intelligence;
		const float* MaxBlockErrorAnglePtr = CombatSettings->MaxBlockAngleErrors.Find(FMath::RoundToInt32(WeaponMastery));
		float BlockErrorAngleMax = MaxBlockErrorAnglePtr ? *MaxBlockErrorAnglePtr : 15.f * (CombatSettings->MaxWeaponMastery * 1.25f - WeaponMastery);
		float ErrorAngle = FMath::RandRange(-BlockErrorAngleMax * IntelligenceBlockErrorFactor, BlockErrorAngleMax * IntelligenceBlockErrorFactor);
		FVector2D ActualBlock = BestBlock.GetRotated(ErrorAngle).GetSafeNormal();
		UE_VLOG(GetOwner(), LogCombat_Block_Npc, Verbose, TEXT("Best block: %s\nActual block: %s"), *BestBlock.ToString(), *ActualBlock.ToString());		
		UE_VLOG(GetOwner(), LogCombat_Block_Npc, Verbose, TEXT("Max error angle: %.2f\nActual error angle: %.2f"), BlockErrorAngleMax, ErrorAngle);		
		UE_VLOG(GetOwner(), LogCombat_Block_Npc, Verbose, TEXT("Intelligence: %.2f\nIntelligence block error factor: %.2f"), Intelligence, IntelligenceBlockErrorFactor);		
	
		PendingBlockInputs.Add(ActualBlock);
	}

	bNpcBlockOnHold = false;
	
	BlockStrengthAccumulationScale = InitialBlockStrengthAccumulationScale;
	float WeaponMasteryParryChance = CombatSettings->AIWeaponMasteryBlockParryChanceDependency.GetRichCurveConst()->HasAnyData() 
		? CombatSettings->AIWeaponMasteryBlockParryChanceDependency.GetRichCurveConst()->Eval(WeaponMastery)
		: WeaponMastery / CombatSettings->MaxWeaponMastery * 1.25f;
	UE_VLOG(GetOwner(), LogCombat_Block_Npc, Verbose, TEXT("Parry chance: %.2f"), WeaponMasteryParryChance);		
	if (FMath::RandRange(0.f, 1.f) > WeaponMasteryParryChance)
	{
		BlockStrengthAccumulationScale *= NonParryingBlockStrengthAccumulationScale;
		UE_VLOG(GetOwner(), LogCombat_Block_Npc, Verbose, TEXT("Decreasing block strength accumulation scale so that block will likely not parry: %.2f vs initial %.2f"),
			BlockStrengthAccumulationScale, InitialBlockStrengthAccumulationScale);		
	}
	
	SetBlockReactionDelay();
#if WITH_EDITOR
	if (DebugOptions.Contains(TEXT("TriggerHitReact_1")))
		Debug_TriggerHitReact();
#endif
}

void UNpcBlockComponent::StartGuidedBlocking()
{
	bUsingExternalBlockInputSource = true;
	ExternalBlockInput = FVector2D::ZeroVector;
	StartBlocking();
}

void UNpcBlockComponent::StartBlocking()
{
	UE_VLOG(GetOwner(), LogCombat_Block_Npc, Log, TEXT("Actually starting blocking after delay"));
	Super::StartBlocking();
#if WITH_EDITOR
	if (DebugOptions.Contains(TEXT("TriggerHitReact_2")))
		Debug_TriggerHitReact();
#endif
}

void UNpcBlockComponent::StopBlocking()
{
	Super::StopBlocking();
	if (auto World = GetWorld())
	{
		auto& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(ReleaseBlockTimer);
		TimerManager.ClearTimer(StartBlockReactionDelayTimer);
	}
}

void UNpcBlockComponent::SetBlockReactionDelay()
{
	float CurrentReactionDelay = 0.f;
	const UMeleeCombatSettings* MeleeCombatSettings = GetDefault<UMeleeCombatSettings>();
	if (auto BlockReactionDelayDependency = MeleeCombatSettings->AIBlockReactionDelayDependency.GetRichCurveConst())
		CurrentReactionDelay += BlockReactionDelayDependency->Eval(NpcCombatant->GetReaction());

	float StaminaRatio = OwnerCombatant->GetStaminaRatio();
	if (auto BlockStaminaDelayDependency = MeleeCombatSettings->AIBlockStaminaDelayDependency.GetRichCurveConst())
		CurrentReactionDelay += BlockStaminaDelayDependency->Eval(StaminaRatio);

	CurrentReactionDelay = FMath::RandRange(CurrentReactionDelay * ReactionDelayScaleMin, CurrentReactionDelay * ReactionDelayScaleMax);
	UE_VLOG(GetOwner(), LogCombat_Block_Npc, Verbose, TEXT("Delaying block start for: %.2f"), CurrentReactionDelay);
	GetWorld()->GetTimerManager().SetTimer(StartBlockReactionDelayTimer, this, &UNpcBlockComponent::StartBlocking, CurrentReactionDelay);
}

void UNpcBlockComponent::ReleaseBlock()
{
	UE_VLOG(GetOwner(), LogCombat_Block_Npc, Log, TEXT("Releasing block"));
	OnNpcFinishedBlockingEvent.ExecuteIfBound();
}

void UNpcBlockComponent::Debug_TriggerHitReact()
{
	auto ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	FGameplayEventData Payload;
	FGameplayAbilityTargetData_ReceivedHit* OwnerData = new FGameplayAbilityTargetData_ReceivedHit();
	OwnerData->HitDirectionTag = CombatGameplayTags::Combat_HitDirection_Front;
	Payload.TargetData.Add(OwnerData);
	ASC->HandleGameplayEvent(CombatGameplayTags::Combat_Ability_HitReact_Event_Activate, &Payload);
}

FVector2D UNpcBlockComponent::GetBlockInput(float DeltaTime) const
{
	return bUsingExternalBlockInputSource 
		? ExternalBlockInput * DeltaTime
		: PendingBlockInputs[CurrentPendingBlockIndex] * BlockInputAccumulationScale * NpcBlockDrawRateScale * DeltaTime;
}

void UNpcBlockComponent::AddBlockInput(const FVector2D& BlockDirectionInput, float DeltaTime)
{
	if (bUsingExternalBlockInputSource)
	{
		Super::AddBlockInput(BlockDirectionInput, DeltaTime);
		return;
	}
	
	if (bNpcBlockOnHold)
	{
		DecayBlock(DeltaTime);
		UE_VLOG(GetOwner(), LogCombat_Block_Npc, VeryVerbose, TEXT("Block on hold - not adding block input"));
		return;
	}
	
	Super::AddBlockInput(BlockDirectionInput, DeltaTime);
	
	// FVector2D AccumulatedBlock = GetAccumulatedBlockVector();
	const float AccumulatedBlockSizeSq = AccumulatedBlock.SizeSquared();
	const float PendingBlockInputSizeSq = PendingBlockInputs[CurrentPendingBlockIndex].SizeSquared();
	UE_VLOG(GetOwner(), LogCombat_Block_Npc, VeryVerbose, TEXT("Checking if %s [%.2f] > %s [%.2f]"), 
		*AccumulatedBlock.ToString(), AccumulatedBlockSizeSq, *PendingBlockInputs[CurrentPendingBlockIndex].ToString(), PendingBlockInputSizeSq);
	if (AccumulatedBlockSizeSq >= PendingBlockInputSizeSq)
	{
		if (CurrentPendingBlockIndex == PendingBlockInputs.Num() - 1)
		{
			bNpcBlockOnHold = true;
			float BlockDuration = FMath::RandRange(NpcHoldBlockDurationMin, NpcHoldBlockDurationMax);
			GetWorld()->GetTimerManager().SetTimer(ReleaseBlockTimer, this, &UNpcBlockComponent::ReleaseBlock, FMath::RandRange(NpcHoldBlockDurationMin, BlockDuration));
			UE_VLOG(GetOwner(), LogCombat_Block_Npc, Log, TEXT("Block draw completed. Setting block held for %.2fs"), BlockDuration);
		}
		else
		{
			CurrentPendingBlockIndex++;
		}
	}
}