// 


#include "UI/ManualBlockOverlayWidget.h"

#include "Components/PlayerManualBlockComponent.h"
#include "Components/TextBlock.h"
#include "UI/AttackHintChipsContainerWidget.h"

void UManualBlockOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OnBlockActiveChanged(false);
}

void UManualBlockOverlayWidget::SetPlayerBlockComponent(UPlayerManualBlockComponent* InPlayerBlockComponent)
{
	PlayerBlockComponent = InPlayerBlockComponent;
	PlayerBlockComponent->OnBlockActiveChangedEvent.AddUObject(this, &UManualBlockOverlayWidget::OnBlockActiveChanged);
	// TODO reconsider reacting to this delegate because player can flood the widget with the block changes
	PlayerBlockComponent->OnBlockAccumulationChangedEvent.AddUObject(this, &UManualBlockOverlayWidget::OnBlockAccumulatedChanged);
	PlayerBlockComponent->OnParryWindowActiveChangedEvent.AddUObject(this, &UManualBlockOverlayWidget::OnParryWindowActiveChanged);
}

void UManualBlockOverlayWidget::NativeDestruct()
{
	if (IsValid(PlayerBlockComponent.Get()))
	{
		PlayerBlockComponent->OnBlockActiveChangedEvent.RemoveAll(this);
		PlayerBlockComponent->OnBlockAccumulationChangedEvent.RemoveAll(this);
		PlayerBlockComponent->OnParryWindowActiveChangedEvent.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UManualBlockOverlayWidget::OnBlockActiveChanged(bool bActive)
{
	SetVisibility(bActive ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	HintChipsWidget->DisableAllChips();
}

void UManualBlockOverlayWidget::OnBlockAccumulatedChanged(FVector2D AccumulationVector, float BlockStrength)
{
	if (IsVisible())
	{
		HintChipsWidget->SetChipActive(AccumulationVector, BlockStrength);
		BlockStrengthTextblock->SetText(FText::AsNumber(BlockStrength));
	}
}

void UManualBlockOverlayWidget::OnParryWindowActiveChanged(bool bActive)
{
	BP_OnParryWindowActiveChanged(bActive);
}
