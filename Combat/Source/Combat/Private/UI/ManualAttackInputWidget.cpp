// 


#include "UI/ManualAttackInputWidget.h"

#include "Components/Border.h"

void UManualAttackInputWidget::SetNextAttackValidity(bool bValid)
{
	NextAttackValidityIndicatorBorder->SetBrushColor(bValid ? NextAttackValidColor : NextAttackInvalidColor);
}

void UManualAttackInputWidget::SetIntermediateState()
{
	NextAttackValidityIndicatorBorder->SetBrushColor(IntermediateColor);
}
