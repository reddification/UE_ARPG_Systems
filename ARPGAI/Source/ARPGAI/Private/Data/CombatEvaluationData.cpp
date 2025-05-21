#include "Data/CombatEvaluationData.h"

using namespace NpcCombatEvaluation;

void FNpcCombatPerceptionData::AddDetectionSource(EDetectionSource NewDetectionSource)
{
	DetectionSource = static_cast<EDetectionSource>(DetectionSource | NewDetectionSource);	
}

bool FNpcCombatPerceptionData::HasDetectionSource(EDetectionSource TestDetectionSource) const
{
	return (DetectionSource & TestDetectionSource) != 0;
}
