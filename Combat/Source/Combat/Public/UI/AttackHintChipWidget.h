// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttackHintChipWidget.generated.h"

/**
 * 
 */
UCLASS()
class COMBAT_API UAttackHintChipWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetChipActive(bool bActive, float Strength);
};
