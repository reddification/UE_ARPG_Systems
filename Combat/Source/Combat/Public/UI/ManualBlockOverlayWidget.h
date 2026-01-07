// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ManualBlockOverlayWidget.generated.h"

class UTextBlock;
class UAttackHintChipsContainerWidget;
class UPlayerManualBlockComponent;
/**
 * 
 */
UCLASS()
class COMBAT_API UManualBlockOverlayWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	void SetPlayerBlockComponent(UPlayerManualBlockComponent* InPlayerBlockComponent);
	virtual void NativeDestruct() override;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnParryWindowActiveChanged(bool bActive);
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipsContainerWidget* HintChipsWidget;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* BlockStrengthTextblock;
	
private:
	TWeakObjectPtr<UPlayerManualBlockComponent> PlayerBlockComponent;
	
	void OnBlockActiveChanged(bool bActive);
	void OnBlockAccumulatedChanged(FVector2D AccumulationVector, float BlockStrength);
	void OnParryWindowActiveChanged(bool bActive);
};
