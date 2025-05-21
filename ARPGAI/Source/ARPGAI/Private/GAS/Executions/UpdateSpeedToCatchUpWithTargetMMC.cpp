// 


#include "GAS/Executions/UpdateSpeedToCatchUpWithTargetMMC.h"
#include "Components/NpcComponent.h"

#include "Interfaces/Npc.h"

float UUpdateSpeedToCatchUpWithTargetMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	auto InstigatorActor = Spec.GetEffectContext().GetInstigator();
	auto Npc = Cast<INpc>(InstigatorActor);
	if (!ensure(Npc))
		return 0.f;

	const AActor* CatchUpTarget = Npc->GetCatchUpTarget();
	auto NpcComponent = InstigatorActor->FindComponentByClass<UNpcComponent>();	
	auto NpcDTR = NpcComponent->GetNpcDTR();
	float MaxSpeedUpScale = 1.5f;
	float MaxSpeed = 1000.f;
	float MinSpeed = 500.f;
	if (NpcDTR->NpcCommonParametersDataAsset)
	{
		MaxSpeedUpScale = NpcDTR->NpcCommonParametersDataAsset->MaxCatchUpSpeedScale;
		MaxSpeed = NpcDTR->NpcCommonParametersDataAsset->MaxSpeed;
		MinSpeed = NpcDTR->NpcCommonParametersDataAsset->MinCatchupSpeed;
	}

	auto CatchUpTargetSpeed = CatchUpTarget->GetVelocity().Size();
	float CatchUpSpeed = FMath::Clamp(CatchUpTargetSpeed * MaxSpeedUpScale, MinSpeed, MaxSpeed);
	return CatchUpSpeed;
}
