// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "NpcStateCategoryWidget.generated.h"

class UVerticalBox;
class UListView;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPGAI_API UNpcStateCategoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	void OnItemChanged(const FGameplayTag& Tag, bool bActive);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText CategoryTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor CategoryColor;
	
protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* CategoryTitleTextblock;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UVerticalBox* ItemsContainer;
	
	TMap<FGameplayTag, UTextBlock*> Items;
};
