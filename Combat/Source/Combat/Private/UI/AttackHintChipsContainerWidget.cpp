// 


#include "UI/AttackHintChipsContainerWidget.h"

#include "Data/CombatDataTypes.h"
#include "UI/AttackHintChipWidget.h"

void UAttackHintChipsContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	AttackTypesToChips = 
	{
		{ EMeleeAttackType::RightMittelhauw, ChipWidget_0 },	
		{ EMeleeAttackType::RightOberhauw, ChipWidget_45 },	
		{ EMeleeAttackType::Thrust, ChipWidget_90 },	
		{ EMeleeAttackType::LeftOberhauw, ChipWidget_135 },	
		{ EMeleeAttackType::LeftMittelhauw, ChipWidget_180 },	
		{ EMeleeAttackType::LeftUnterhauw, ChipWidget_225 },	
		{ EMeleeAttackType::VerticalSlash, ChipWidget_270 },	
		{ EMeleeAttackType::RightUnterhauw, ChipWidget_315 },	
	};
}

void UAttackHintChipsContainerWidget::SetChipActive(EMeleeAttackType AttackType)
{
	if (bMirrorAttack)
		AttackType = MirrorAttack(AttackType);
	
	for (const auto& Chip : AttackTypesToChips)
		Chip.Value->BP_SetChipActive(Chip.Key == AttackType, Chip.Key == AttackType ? 1.f : 0.f);
}

void UAttackHintChipsContainerWidget::SetChipActive(FVector2D AccumulationVector, float Strength)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UAttackHintChipsContainerWidget::SetChipActive)
	
	const FVector2D AccumulatedVectorDirection = AccumulationVector.GetSafeNormal();
	EMeleeAttackType AttackType = EMeleeAttackType::RightMittelhauw;
	bool bUpper = (AccumulatedVectorDirection | FVector2D(0, 1)) >= 0.f;
	float InputRadians = FMath::Acos(AccumulatedVectorDirection | FVector2D(1.f, 0));
	if (!bUpper)
		InputRadians = TWO_PI - InputRadians;
	
	if (bMirrorAttack)
		InputRadians = bUpper ? PI - InputRadians : 3.f * PI - InputRadians;
	
	TArray<EMeleeAttackType> AttackTypesOrderUpper = 
	{
		EMeleeAttackType::RightMittelhauw, EMeleeAttackType::RightOberhauw, bMirrorAttack ? EMeleeAttackType::VerticalSlash : EMeleeAttackType::Thrust,
		EMeleeAttackType::LeftOberhauw, EMeleeAttackType::LeftMittelhauw
	};
	
	TArray<EMeleeAttackType> AttackTypesOrderLower = 
	{
		EMeleeAttackType::RightMittelhauw, EMeleeAttackType::RightUnterhauw, bMirrorAttack ? EMeleeAttackType::Thrust : EMeleeAttackType::VerticalSlash,
		EMeleeAttackType::LeftUnterhauw, EMeleeAttackType::LeftMittelhauw
	};
	
	float r = 0.f;
	for (int i = 0; i < 5; i++, r+= PI / 4.f)
	{
		if (bUpper && FMath::IsNearlyEqual(r, InputRadians, PI / 8.f))
		{
			AttackType = AttackTypesOrderUpper[i];
			break;
		}
		else if (!bUpper && FMath::IsNearlyEqual(TWO_PI - r, InputRadians, PI / 8.f))
		{
			AttackType = AttackTypesOrderLower[i];
			break;
		}
	}
	
	for (const auto& Chip : AttackTypesToChips)
		Chip.Value->BP_SetChipActive(Chip.Key == AttackType, Chip.Key == AttackType ? Strength : 0.f);
}

void UAttackHintChipsContainerWidget::DisableAllChips()
{
	for (const auto& Chip : AttackTypesToChips)
		Chip.Value->BP_SetChipActive(false, 0.f);
}

EMeleeAttackType UAttackHintChipsContainerWidget::MirrorAttack(EMeleeAttackType AttackType) const
{
	switch (AttackType)
	{
		case EMeleeAttackType::RightMittelhauw:
			return EMeleeAttackType::LeftMittelhauw;
		case EMeleeAttackType::RightOberhauw:
			return EMeleeAttackType::LeftOberhauw;
		case EMeleeAttackType::Thrust:
			return EMeleeAttackType::VerticalSlash;
		case EMeleeAttackType::LeftOberhauw:
			return EMeleeAttackType::RightOberhauw;
		case EMeleeAttackType::LeftMittelhauw:
			return EMeleeAttackType::RightMittelhauw;
		case EMeleeAttackType::LeftUnterhauw:
			return EMeleeAttackType::RightUnterhauw;
		case EMeleeAttackType::VerticalSlash:
			return EMeleeAttackType::Thrust;
		case EMeleeAttackType::RightUnterhauw:
			return EMeleeAttackType::LeftUnterhauw;
		case EMeleeAttackType::SpinRightMittelhauw:
			return EMeleeAttackType::LeftMittelhauw;
		case EMeleeAttackType::SpinRightOberhauw:
			return EMeleeAttackType::LeftOberhauw;
		case EMeleeAttackType::SpinLeftMittelhauw:
			return EMeleeAttackType::RightMittelhauw;
		case EMeleeAttackType::SpinLeftOberhauw:
			return EMeleeAttackType::RightOberhauw;
		default:
			return AttackType;
	}
}
