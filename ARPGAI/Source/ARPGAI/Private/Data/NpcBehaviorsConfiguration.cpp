#include "Data/NpcBehaviorsConfiguration.h"

#include "BehaviorEvaluators/v2/BehaviorEvaluator_OperationBased.h"

void UNpcBehaviorsConfiguration::GenerateFormulasDescriptions()
{
	for (auto* BehaviorEvaluator : BehaviorEvaluators)
		if (auto OperationsBased = Cast<UBehaviorEvaluatorConfig_OperationBased>(BehaviorEvaluator))
			OperationsBased->GenerateFormulasDescriptions();
}
