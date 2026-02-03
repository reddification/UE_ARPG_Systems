// 


#include "GAS/Executions/UpdateSpeedInCombatMMC.h"

#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Components/NpcInterfaceComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/Npc.h"

UUpdateSpeedInCombatMMC::UUpdateSpeedInCombatMMC()
{
	DistanceToTargetDef.AttributeToCapture = UNpcCombatAttributeSet::GetDistanceToTargetAttribute();
	DistanceToTargetDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	DistanceToTargetDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(DistanceToTargetDef);
}

float UUpdateSpeedInCombatMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	if (InstigatorActor == nullptr)
		return 0.f;
	
	auto Npc = Cast<INpc>(InstigatorActor);
	if (Npc == nullptr)
		return 0.f;
	
	const float CurrentMoveSpeed = Npc->GetMoveSpeed();
	auto NpcInterfaceComponent = InstigatorActor->FindComponentByClass<UNpcInterfaceComponent>();
	if (!ensure(NpcInterfaceComponent))
		return bReturnScale ? 1.f : CurrentMoveSpeed;

	auto NpcCombatLogicComponent = InstigatorActor->FindComponentByClass<UNpcCombatLogicComponent>();	
	auto NpcComponent = InstigatorActor->FindComponentByClass<UNpcComponent>();	

	float DistanceToTarget = NpcCombatLogicComponent->GetDistanceToTarget();
	float BaseSpeed = NpcInterfaceComponent->GetBaseMoveSpeed();
	const FGameplayTag& MovementPaceType = NpcComponent->GetMovementPaceType();
	if (!MovementPaceType.IsValid())
		return bReturnScale ? 1.f : CurrentMoveSpeed;
	
	float RequiredSpeed = BaseSpeed;
	auto NpcCombatParameters = NpcCombatLogicComponent->GetNpcCombatParameters();
	if (!ensure(NpcCombatParameters))
		return bReturnScale ? 1.f : CurrentMoveSpeed;

	if (const auto* MoveSpeedDependency = NpcCombatParameters->NpcCombatEvaluationParameters.MoveSpeedDependencies.Find(MovementPaceType))
	{
		if (auto Dependency = MoveSpeedDependency->DistanceToTargetToMoveSpeedDependency.GetRichCurveConst())
			RequiredSpeed = Dependency->Eval(DistanceToTarget);
	}
	else
	{
		UE_VLOG(NpcComponent->GetOwner(), LogARPGAI, Warning, TEXT("No move speed dependency found for tag %s"), *MovementPaceType.ToString());
	}
	
	return bReturnScale ? RequiredSpeed / BaseSpeed : RequiredSpeed;
}
