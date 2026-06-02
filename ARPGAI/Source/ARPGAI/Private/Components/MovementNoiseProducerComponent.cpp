#include "Components/MovementNoiseProducerComponent.h"

#include "Data/AIGameplayTags.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcMovementNoiseProducer.h"
#include "Perception/AISense_Hearing.h"

UMovementNoiseProducerComponent::UMovementNoiseProducerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.2f;
}

// Called when the game starts
void UMovementNoiseProducerComponent::BeginPlay()
{
	Super::BeginPlay();
	auto NoiseProducerInterface = Cast<INpcMovementNoiseProducer>(GetOwner());
	if (NoiseProducerInterface != nullptr && ensure(IsValid(Config)))
	{
		NoiseProducer.SetObject(GetOwner());
		NoiseProducer.SetInterface(NoiseProducerInterface);
		FootstepTag = AIGameplayTags::AI_Noise_Footstep.GetTag().GetTagName();
		SetComponentTickEnabled(true);
	}
}

void UMovementNoiseProducerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!ensure(Config))
		return;
	
	if (!NoiseProducer->CanBeHeardMoving_NPC())
	{
		NoiseAccumulator = FMath::Max(NoiseAccumulator - Config->DecayRate * DeltaTime, 0.f);
		return;
	}
	
	const float SpeedNoise = Config->SpeedToNoiseDependency.GetRichCurveConst()->Eval(NoiseProducer->GetSpeed_NPC());
	const float WeightScale = Config->CarriedWeightToNoiseDependency.GetRichCurveConst()->Eval(NoiseProducer->GetCarriedWeight_NPC());
	const float DexterityScale = Config->DexterityToNoiseDependency.GetRichCurveConst()->Eval(NoiseProducer->GetDexterity_NPC());
	const float SurfaceScale = 1.f; // 24 Apr 2026 (aki) TODO. Currently it's just an idea
	
	const float TotalNoiseAccumulation = SpeedNoise * WeightScale * DexterityScale * SurfaceScale;
	
	NoiseAccumulator += TotalNoiseAccumulation * DeltaTime * Config->AccumulationRate;

	if (bShowDebug)
	{
		GEngine->AddOnScreenDebugMessage(1, 3.f, FColorList::LimeGreen, FString::Printf(TEXT("Speed noise = %.2f"), SpeedNoise));
		GEngine->AddOnScreenDebugMessage(2, 3.f, FColorList::GreenYellow, FString::Printf(TEXT("Weight scale = %.2f"), WeightScale));
		GEngine->AddOnScreenDebugMessage(3, 3.f, FColorList::GreenCopper, FString::Printf(TEXT("Dexterity scale = %.2f"), DexterityScale));
		GEngine->AddOnScreenDebugMessage(4, 3.f, FColorList::MandarianOrange, FString::Printf(TEXT("This frame buildup = %.2f"), TotalNoiseAccumulation));
		GEngine->AddOnScreenDebugMessage(5, 3.f, FColorList::OrangeRed, FString::Printf(TEXT("Total accumulation = %.2f"), NoiseAccumulator));
	}
	
	if (NoiseAccumulator > Config->NoiseAccumulationThreshold)
	{
		EmitNoise(TotalNoiseAccumulation);
		NoiseAccumulator = 0.f;
	}
}

void UMovementNoiseProducerComponent::EmitNoise(float Loudness) const
{
	UE_VLOG_LOCATION(GetOwner(), LogARPGAI_Perception, VeryVerbose, GetOwner()->GetActorLocation(), 20, FColorList::Yellow, TEXT("Accumulated Movement Noise"));
	const float Range = Config->LoudnessToSoundRangeDependency.GetRichCurveConst()->Eval(Loudness);
	UE_VLOG(GetOwner(), LogARPGAI_Perception, VeryVerbose, TEXT("UMovementNoiseProducerComponent: Reporting accumulated noise. Loudness = %.2f, range = %.2f"),
		Loudness, Range);
	
	if (bShowDebug)
	{
		GEngine->AddOnScreenDebugMessage(6, 1.f, FColorList::BlueViolet,
			FString::Printf(TEXT("Emitting noise with range = %.2f"), Range));
	}
	
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetOwner()->GetActorLocation(), Loudness, GetOwner(),
		Range, FootstepTag);
}