// 


#include "GAS/Executions/UpdateSpeedInCombatMMC.h"

#include "Components/NpcCombatLogicComponent.h"
#include "Components/NpcComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/Npc.h"

float UUpdateSpeedInCombatMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	auto Npc = Cast<INpc>(InstigatorActor);
	if (!ensure(Npc))
		return 0.f;

	auto NpcCombatLogicComponent = InstigatorActor->FindComponentByClass<UNpcCombatLogicComponent>();	
	auto NpcComponent = InstigatorActor->FindComponentByClass<UNpcComponent>();	

	float DistanceToTarget = NpcCombatLogicComponent->GetDistanceToTarget();
	float NewSpeed = Npc->GetMoveSpeed();
	const FGameplayTag& MovementPaceType = NpcComponent->GetMovementPaceType();
	if (!MovementPaceType.IsValid())
		return NewSpeed;
	
	auto NpcDTR = NpcComponent->GetNpcDTR();
	auto MoveSpeedDependency = NpcDTR->NpcCombatParametersDataAsset->NpcCombatEvaluationParameters.MoveSpeedDependencies.Find(MovementPaceType);
	if (MoveSpeedDependency)
	{
		if (auto Dependency = MoveSpeedDependency->DistanceToTargetToMoveSpeedDependency.GetRichCurve())
		{
			NewSpeed = Dependency->Eval(DistanceToTarget);
		}
	}
	else
	{
		UE_VLOG(NpcComponent->GetOwner(), LogARPGAI, Warning, TEXT("No move speed dependencty found for tag %s"), *MovementPaceType.ToString());
	}
	
	return NewSpeed;
}
