// 


#include "UI/NpcAttackDirectionHintWidget.h"

#include "Components/NpcMeleeCombatComponent.h"
#include "Data/CombatLogChannels.h"
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
	NpcCombatComponent->OnAttackActivePhaseChanged.AddUObject(this, &UNpcAttackDirectionHintWidget::OnAttackPhaseChanged);
	NpcCombatComponent->OnAttackEndedEvent.AddUObject(this, &UNpcAttackDirectionHintWidget::OnAttackCompleted);
	NpcCombatComponent->OnAttackFeintedEvent.AddUObject(this, &UNpcAttackDirectionHintWidget::OnAttackCompleted);
}

void UNpcAttackDirectionHintWidget::OnAttackPhaseChanged(EMeleeAttackPhase OldAttackPhase, EMeleeAttackPhase NewAttackPhase)
{
	bool bVisible = NewAttackPhase == EMeleeAttackPhase::WindUp || NewAttackPhase == EMeleeAttackPhase::Release;
	SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	
#if WITH_EDITOR
	auto AttackPhaseEnum = StaticEnum<EMeleeAttackPhase>();
	UE_VLOG(NpcCombatComponent->GetOwner(), LogCombat_UI, Verbose, TEXT("Attack phase changed from %s to %s. New visibility state = %s"),
		*AttackPhaseEnum->GetDisplayValueAsText(OldAttackPhase).ToString(), *AttackPhaseEnum->GetDisplayValueAsText(NewAttackPhase).ToString(),
		bVisible ? TEXT("Visible") : TEXT("Non-Visible"));
#endif
}

void UNpcAttackDirectionHintWidget::SetCombatComponent(UNpcMeleeCombatComponent* InMeleeCombatComponent)
{
	if (InMeleeCombatComponent)
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
		NpcCombatComponent->OnAttackActivePhaseChanged.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UNpcAttackDirectionHintWidget::SetPreparedAttack(EMeleeAttackType AttackType)
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ChipsWidget->SetChipActive(AttackType);
	UE_VLOG(NpcCombatComponent->GetOwner(), LogCombat_UI, Verbose, TEXT("Showing NPC attack hint widget"));
}

void UNpcAttackDirectionHintWidget::OnAttackCompleted()
{
	SetVisibility(ESlateVisibility::Collapsed);
	UE_VLOG(NpcCombatComponent->GetOwner(), LogCombat_UI, Verbose, TEXT("Hiding NPC attack hint widget"));
}
