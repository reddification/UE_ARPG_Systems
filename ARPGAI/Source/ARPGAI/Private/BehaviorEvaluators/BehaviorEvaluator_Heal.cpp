#include "BehaviorEvaluators/BehaviorEvaluator_Heal.h"

#include "AIController.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Data/LogChannels.h"
#include "Interfaces/NpcAliveActor.h"

TUniquePtr<FBehaviorEvaluator_Base> UBehaviorEvaluatorConfig_Heal::CreateEvaluator(UBehaviorTreeComponent* BTComponent) const
{
	return MakeUnique<FBehaviorEvaluator_Heal>(*BTComponent, this);
}

FBehaviorEvaluator_Heal::FBehaviorEvaluator_Heal(UBehaviorTreeComponent& OwnerComp,
	const UBehaviorEvaluatorConfig_Base* Config) : Super(OwnerComp, Config)
{
	HealConfig = Cast<UBehaviorEvaluatorConfig_Heal>(Config);
}

void FBehaviorEvaluator_Heal::Update(const float DeltaTime)
{
	Super::Update(DeltaTime);
	const float RegressionOffset = GetUtilityOffset();

	auto OwnerAliveCreature = Cast<INpcAliveActor>(Pawn.Get());
	
	float Desire = 0.f;
	const float NormalizedHealth = OwnerAliveCreature->GetHealth_NPC() / OwnerAliveCreature->GetMaxHealth_NPC();
	if (auto Dependency = HealConfig->NormalizedHealthToDesireDependency.GetRichCurveConst())
		Desire = Dependency->Eval(NormalizedHealth);
	
	UE_VLOG(AIController.Get(), LogAI_Heal, Verbose, TEXT("Heal health-based (%.2f) desire = %.2f"),
		NormalizedHealth, Desire);
	
	float Reluctance = 0.f;
	if (Desire > 0.f)
	{
		const auto& PerceptionData = PerceptionComponent->GetShortTermCharactersMemory();
		const auto* ReluctanceDistanceDependency = HealConfig->DistanceToEnemiesReluctanceDependency.GetRichCurveConst();
		for (const auto& PerceptionItem : PerceptionData)
		{
			bool bConsiderableEnemy = PerceptionItem.Value.IsHostile() && PerceptionItem.Value.IsAlive() && PerceptionItem.Value.DotProduct_ActorFV_ToOwner > 0.92f;
			if (!bConsiderableEnemy)
				continue;
		
			const float IndividualReluctance = ReluctanceDistanceDependency->Eval(PerceptionItem.Value.Distance);
			Reluctance += IndividualReluctance;
			
#if WITH_EDITOR
			UE_VLOG(AIController.Get(), LogAI_Heal, Verbose, TEXT("Heal distance-based reluctance = %.2f. From %s"),
				IndividualReluctance, *PerceptionItem.Value.CharacterId.ToString());
			UE_VLOG_CAPSULE(AIController.Get(), LogAI_Heal, VeryVerbose,
				PerceptionItem.Key->GetActorLocation() - FVector::UpVector * 90.f, 90.f, 30.f, FQuat::Identity, 
				FColorList::DarkOrchid, TEXT("Heal reluctance: %.2f [%s]"), IndividualReluctance, *PerceptionItem.Value.CharacterId.ToString());
#endif
		}
	
		UE_VLOG(AIController.Get(), LogAI_Heal, Verbose, TEXT("Heal total reluctance = %.2f"), Reluctance);
	}
	
	InterpolateUtility(RegressionOffset + Desire - Reluctance, DeltaTime);
}