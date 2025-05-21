// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "UObject/Object.h"
#include "NpcStateWidget.generated.h"

class USizeBox;
class UNpcStateCategoryWidget;
class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class ARPGAI_API UNpcStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetNPC(ACharacter* InOwnerCharacter);

	void SetHostile(bool bHostile);

	void SetDetailedNpcView(bool bDetailed);
	
protected:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UProgressBar* AggressionBar;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UProgressBar* StaminaBar;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UProgressBar* PoiseBar;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* HealthTextBlock;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* AggressionTextBlock;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* PoiseTextBlock;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* StaminaTextBlock;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* NameTextblock;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	USizeBox* BarsSizeBox;
	
	// UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	// UNpcStateCategoryWidget* ActiveAbilityContainer;
	//
	// UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	// UNpcStateCategoryWidget* ActiveStateContainer;
	//
	// UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	// UNpcStateCategoryWidget* ActiveBehaviorContainer;
	
private:
	void SetStatWidgetData(UTextBlock* TextBlock, UProgressBar* ProgressBar, float Value, float MaxValue);

	void OnNpcHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnNpcMaxHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnAggressionChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void MaxOnAggressionChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnPoiseChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnMaxPoiseChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	
	void OnStateChanged(const FGameplayTag& StateTag, bool bActive);
	void OnBehaviorChanged(const FGameplayTag& BehaviorTag, bool bActive);
	void OnActiveAbilityChanged(const FGameplayTag& AbilityTag, bool bActive);
	void UpdateStateWidgets(UNpcStateCategoryWidget* ContainerWidget, const FGameplayTag& Tag, bool bActive);

	void UpdateHostileState();
	
	TWeakObjectPtr<ACharacter> OwnerCharacter;

	float Health = 0.f;
	float MaxHealth = 0.f;
	float Aggression = 0.f;
	float MaxAggression = 0.f;
	float Poise = 0.f;
	float MaxPoise = 0.f;
	float Stamina = 0.f;
	float MaxStamina = 0.f;

	bool bHostile = false;
};