// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CombatDataTypes.h"
#include "NpcAttackDirectionHintWidget.generated.h"

class UAttackHintChipsContainerWidget;
class UAttackHintChipWidget;
class UNpcMeleeCombatComponent;
/**
 * 
 */
UCLASS()
class COMBAT_API UNpcAttackDirectionHintWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetCombatComponent(UNpcMeleeCombatComponent* InMeleeCombatComponent);
	
	UPROPERTY(meta=(BindWidget))
	UAttackHintChipsContainerWidget* ChipsWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ECollisionChannel> TraceTestChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DistanceThreshold = 450.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DotProductThreshold = 0.935f;
	
private:
	void OnAttackPhaseChanged(EMeleeAttackPhase OldAttackPhase, EMeleeAttackPhase NewAttackPhase);
	void InitializeNpcCombatComponent();
	void SetPreparedAttack(EMeleeAttackType AttackType);
	void OnAttackCompleted();
	
	TWeakObjectPtr<UNpcMeleeCombatComponent> NpcCombatComponent;
};
