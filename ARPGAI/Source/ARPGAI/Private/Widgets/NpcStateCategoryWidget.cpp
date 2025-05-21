// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/NpcStateCategoryWidget.h"

#include "CommonTextBlock.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UNpcStateCategoryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	CategoryTitleTextblock->SetText(CategoryTitle);
	CategoryTitleTextblock->SetColorAndOpacity(CategoryColor);
}

void UNpcStateCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ItemsContainer->ClearChildren();
}

void UNpcStateCategoryWidget::OnItemChanged(const FGameplayTag& Tag, bool bActive)
{
	if (bActive && Tag.IsValid() && !Items.Contains(Tag))
	{
		UTextBlock* NewTextBlock = WidgetTree->ConstructWidget<UTextBlock>();
		// CreateWidget<UTextBlock>(this, UTextBlock::StaticClass());
		// FSlateFontInfo FontInfo;
		NewTextBlock->SetColorAndOpacity(CategoryColor);
		NewTextBlock->SetText(FText::FromString(Tag.ToString()));
		NewTextBlock->SetAutoWrapText(true);
		auto FontInfo = NewTextBlock->GetFont();
		FontInfo.Size = 14;
		FontInfo.OutlineSettings.OutlineSize = 1;
		FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
		NewTextBlock->SetFont(FontInfo);

		Items.Add(Tag, NewTextBlock);
		auto AddedText = ItemsContainer->AddChildToVerticalBox(NewTextBlock);
		AddedText->SetHorizontalAlignment(HAlign_Fill);
	}
	else
	{
		if (UTextBlock** Item = Items.Find(Tag))
		{
			ItemsContainer->RemoveChild(*Item);
			(*Item)->RemoveFromParent();
			Items.Remove(Tag);
		}
	}

	SetVisibility(Items.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}
