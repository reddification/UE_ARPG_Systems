// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CombatDataTypes.h"
#include "ManualAttackOverlayWidget.generated.h"

class UImage;
class UPlayerSwingControlCombatComponent;
class UManualAttackInputWidget;
/**
 * 
 */
UCLASS()
class COMBAT_API UManualAttackOverlayWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializeAttackOverlay(UPlayerSwingControlCombatComponent* InCombatComponent);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EAttackStepDirection, float> AttackStepToImageRotationAngles;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HideOverlayDelay = 1.f;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_0;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_45;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_90;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_135;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_180;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_225;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_270;
	
	UPROPERTY(meta=(BindWidget))
	UManualAttackInputWidget* AttackProgressBar_315;
	
	UPROPERTY(meta=(BindWidget))
	UImage* StepDirectionImage;
	
private:
	TMap<EMeleeAttackType, TWeakObjectPtr<UManualAttackInputWidget>> AttackTypesToProgressBars;
	TWeakObjectPtr<UPlayerSwingControlCombatComponent> CombatComponent;
	FTimerHandle HideOverlayTimer;
	
	void SetAttackInputProgress(EMeleeAttackType AttackType, float Progress);
	void SetAttackStepDirection(EAttackStepDirection AttackStepDirection);
	void SetValidNextAttacks(const TSet<EMeleeAttackType>& ValidNextAttacks);
	void SetAttackRegistering(bool bRegistering);
	void HideOverlay();
	
	void OnAttackStarted(EMeleeAttackType AttackType);
	void OnAttackEnded();
};
