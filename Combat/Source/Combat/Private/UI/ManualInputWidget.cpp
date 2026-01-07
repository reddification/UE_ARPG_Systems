// 


#include "UI/ManualInputWidget.h"

#include "Components/ProgressBar.h"
#include "Curves/CurveLinearColor.h"
#include "Data/CombatLogChannels.h"

void UManualInputWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetProgress(0);
}

void UManualInputWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bDecaying)
		SetProgress(FMath::Max(0.f, InputProgressBar->GetPercent() - InDeltaTime * DecayRate));
}

void UManualInputWidget::SetProgress(float NewProgress)
{
	if (NewProgress < 0.0f || NewProgress > 1.0f)
	{
		UE_LOG(LogCombat, Log, TEXT("Input Widget %s: Incorrect new input value: %2.f"), *GetName(), NewProgress);
		NewProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);
	}
	
	float OldValue = InputProgressBar->GetPercent();
	InputProgressBar->SetPercent(NewProgress);
	InputProgressBar->SetFillColorAndOpacity(ProgressBarColorCurve->GetLinearColorValue(NewProgress));
	auto& TimerManager = GetWorld()->GetTimerManager();
	if (NewProgress > OldValue)
	{
		bDecaying = false;
		TimerManager.SetTimer(DecayActivationTimer, this, &UManualInputWidget::StartDecay, RegressDelay, false);
		UE_LOG(LogCombat, Log, TEXT("Input Widget %s: Input increased, restarting decay timer"), *GetName());
	}
	else if (FMath::IsNearlyZero(NewProgress))
	{
		bDecaying = false;
		TimerManager.ClearTimer(DecayActivationTimer);
		UE_LOG(LogCombat, Log, TEXT("Input Widget %s: regressed to zero, disabling decay"), *GetName());
	}
	
	TRange<float> InputOpacityRange(0.f, 0.5f);
	TRange<float> OutputOpacityRange(0.2f, 0.8f);
	float NewOpacity = FMath::GetMappedRangeValueClamped(InputOpacityRange, OutputOpacityRange, NewProgress);
	UE_LOG(LogCombat, Log, TEXT("Input Widget %s: opacity = %.2f"), *GetName(), NewOpacity);
	SetRenderOpacity(NewOpacity);
}

void UManualInputWidget::StartDecay()
{
	bDecaying = true;
	UE_LOG(LogCombat, Log, TEXT("Input Widget %s: start decaying"), *GetName());
}
