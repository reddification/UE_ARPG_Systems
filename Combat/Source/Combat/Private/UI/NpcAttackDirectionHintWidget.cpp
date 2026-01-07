// 


#include "UI/NpcAttackDirectionHintWidget.h"

#include "Components/NpcMeleeCombatComponent.h"
#include "UI/AttackHintChipsContainerWidget.h"

void UNpcAttackDirectionHintWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
	
	// when the widget above NPCs head is hidden (and it's hidden when NPC is no longer in player POV),
	// NativeDestruct is triggered, but it seems that widget's UObject is not destroyed, because when the player sees this NPC again
	// NativeConstruct is triggered but the NpcCombatComponent is valid. So we have to reinitialize it
	if (NpcCombatComponent.IsValid()) 
		InitializeNpcCombatComponent();
}

void UNpcAttackDirectionHintWidget::InitializeNpcCombatComponent()
{
	NpcCombatComponent->OnAttackStartedEvent.AddUObject(this, &UNpcAttackDirectionHintWidget::SetPreparedAttack);
	NpcCombatComponent->OnAttackEndedEvent.AddUObject(this, &UNpcAttackDirectionHintWidget::OnAttackCompleted);
	NpcCombatComponent->OnAttackFeintedEvent.AddUObject(this, &UNpcAttackDirectionHintWidget::OnAttackCompleted);
}

void UNpcAttackDirectionHintWidget::SetCombatComponent(UNpcMeleeCombatComponent* InMeleeCombatComponent)
{
	if (ensure(InMeleeCombatComponent))
	{
		NpcCombatComponent = InMeleeCombatComponent;
		InitializeNpcCombatComponent();
	}
}

void UNpcAttackDirectionHintWidget::NativeDestruct()
{
	if (NpcCombatComponent.IsValid() && IsValid(NpcCombatComponent.Get()))
	{
		NpcCombatComponent->OnAttackStartedEvent.RemoveAll(this);
		NpcCombatComponent->OnAttackEndedEvent.RemoveAll(this);
		NpcCombatComponent->OnAttackFeintedEvent.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UNpcAttackDirectionHintWidget::SetPreparedAttack(EMeleeAttackType AttackType)
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ChipsWidget->SetChipActive(AttackType);
}

void UNpcAttackDirectionHintWidget::OnAttackCompleted()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
