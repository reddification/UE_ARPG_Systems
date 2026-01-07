// 


#include "UI/ManualAttackOverlayWidget.h"

#include "Components/Image.h"
#include "Components/PlayerSwingControlCombatComponent.h"
#include "UI/ManualAttackInputWidget.h"

void UManualAttackOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HideOverlay();
	AttackTypesToProgressBars = 
	{
		{ EMeleeAttackType::RightMittelhauw, AttackProgressBar_0 },  	
		{ EMeleeAttackType::RightOberhauw, AttackProgressBar_45 },  	
		{ EMeleeAttackType::Thrust, AttackProgressBar_90 },  	
		{ EMeleeAttackType::LeftOberhauw, AttackProgressBar_135 },  	
		{ EMeleeAttackType::LeftMittelhauw, AttackProgressBar_180 },  	
		{ EMeleeAttackType::LeftUnterhauw, AttackProgressBar_225 },  	
		{ EMeleeAttackType::VerticalSlash, AttackProgressBar_270 },  	
		{ EMeleeAttackType::RightUnterhauw, AttackProgressBar_315 },  	
	};
}

void UManualAttackOverlayWidget::NativeDestruct()
{
	if (IsValid(CombatComponent.Get()))
	{
		CombatComponent->OnAttackStartedEvent.RemoveAll(this);
		CombatComponent->OnAttackEndedEvent.RemoveAll(this);
		CombatComponent->AttackInputProgressUpdatedEvent.RemoveAll(this);
		CombatComponent->AttackStepDirectionUpdatedEvent.RemoveAll(this);
		CombatComponent->RegisteringAttackStateChangedEvent.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UManualAttackOverlayWidget::InitializeAttackOverlay(UPlayerSwingControlCombatComponent* InCombatComponent)
{
	CombatComponent = InCombatComponent;
	CombatComponent->OnAttackStartedEvent.AddUObject(this, &UManualAttackOverlayWidget::OnAttackStarted);
	CombatComponent->OnAttackEndedEvent.AddUObject(this, &UManualAttackOverlayWidget::OnAttackEnded);
	CombatComponent->AttackInputProgressUpdatedEvent.AddUObject(this, &UManualAttackOverlayWidget::SetAttackInputProgress);
	CombatComponent->AttackStepDirectionUpdatedEvent.AddUObject(this, &UManualAttackOverlayWidget::SetAttackStepDirection);
	CombatComponent->RegisteringAttackStateChangedEvent.AddUObject(this, &UManualAttackOverlayWidget::SetAttackRegistering);
}

void UManualAttackOverlayWidget::SetAttackInputProgress(EMeleeAttackType AttackType, float Progress)
{
	if (AttackType == EMeleeAttackType::None)
		return;
	
	auto ProgressBarPtr = AttackTypesToProgressBars.Find(AttackType);
	if (ensure(ProgressBarPtr))
	{
		auto ProgressBar = ProgressBarPtr->Get(); 
		ProgressBar->SetProgress(Progress);
	}
}

void UManualAttackOverlayWidget::SetValidNextAttacks(const TSet<EMeleeAttackType>& ValidNextAttacks)
{
	for (const auto& ProgressBar : AttackTypesToProgressBars)
		ProgressBar.Value->SetNextAttackValidity(ValidNextAttacks.Contains(ProgressBar.Key));
}

void UManualAttackOverlayWidget::SetAttackRegistering(bool bRegistering)
{
	auto& TimerManager = GetWorld()->GetTimerManager();
	if (bRegistering)
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		TimerManager.ClearTimer(HideOverlayTimer);
	}
	else
	{
		TimerManager.SetTimer(HideOverlayTimer, this, &UManualAttackOverlayWidget::HideOverlay, HideOverlayDelay, false);
	}
}

void UManualAttackOverlayWidget::HideOverlay()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UManualAttackOverlayWidget::SetAttackStepDirection(EAttackStepDirection AttackStepDirection)
{
	if (AttackStepDirection == EAttackStepDirection::None)
	{
		StepDirectionImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		float* ImageRotationAngle = AttackStepToImageRotationAngles.Find(AttackStepDirection);
		if (ensure(ImageRotationAngle))
		{
			StepDirectionImage->SetRenderTransformAngle(*ImageRotationAngle);
			StepDirectionImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void UManualAttackOverlayWidget::OnAttackStarted(EMeleeAttackType AttackType)
{
	// TODO after refactoring combat from ABP to FlowGraph, get valid next attack types and send them to the widget
	TSet<EMeleeAttackType> ValidNextAttacks;
	for (const auto& ProgressBar : AttackTypesToProgressBars)
	{
		ProgressBar.Value->SetProgress(0.f);
		ProgressBar.Value->SetNextAttackValidity(ValidNextAttacks.Contains(ProgressBar.Key));
	}
}

void UManualAttackOverlayWidget::OnAttackEnded()
{
	for (const auto& ProgressBar : AttackTypesToProgressBars)
		ProgressBar.Value->SetIntermediateState();
}
