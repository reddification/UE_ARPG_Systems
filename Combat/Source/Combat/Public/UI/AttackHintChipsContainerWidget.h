// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttackHintChipsContainerWidget.generated.h"

class UUniformGridPanel;
enum class EMeleeAttackType : uint8;
class UAttackHintChipWidget;
/**
 * 
 */
UCLASS()
class COMBAT_API UAttackHintChipsContainerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	void SetChipActive(EMeleeAttackType AttackType);
	void SetChipActive(FVector2D AccumulationVector, float Strength);
	void DisableAllChips();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bMirrorAttack = false;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_0;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_45;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_90;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_135;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_180;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_225;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_270;
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipWidget* ChipWidget_315;
	
	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* ChipsGrid;

private:
	TMap<EMeleeAttackType, TWeakObjectPtr<UAttackHintChipWidget>> AttackTypesToChips;
	EMeleeAttackType MirrorAttack(EMeleeAttackType AttackType) const;
};
