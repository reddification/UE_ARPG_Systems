// 

#pragma once

#include "CoreMinimal.h"
#include "ManualInputWidget.h"
#include "ManualAttackInputWidget.generated.h"

class UBorder;
/**
 * 
 */
UCLASS()
class COMBAT_API UManualAttackInputWidget : public UManualInputWidget
{
	GENERATED_BODY()
	
public:
	void SetNextAttackValidity(bool bValid);
	void SetIntermediateState();
	
protected:
	UPROPERTY(meta=(BindWidget))
	UBorder* NextAttackValidityIndicatorBorder;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor NextAttackValidColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor NextAttackInvalidColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor IntermediateColor;
};
