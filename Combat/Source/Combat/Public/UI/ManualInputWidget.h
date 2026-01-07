// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ManualInputWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class COMBAT_API UManualInputWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetProgress(float NewProgress);
	
protected:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* InputProgressBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveLinearColor* ProgressBarColorCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DecayRate = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RegressDelay = 0.3f;

private:
	FTimerHandle DecayActivationTimer;
	bool bDecaying = false;
	
	void StartDecay();
};
