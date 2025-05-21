// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/NpcStateWidget.h"

#include "AbilitySystemInterface.h"
#include "Components/NpcComponent.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "Interfaces/NpcAliveCreature.h"
#include "Interfaces/Npc.h"
#include "Widgets/NpcStateCategoryWidget.h"

void UNpcStateWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// ActiveAbilityContainer->SetVisibility(ESlateVisibility::Hidden);
	// ActiveBehaviorContainer->SetVisibility(ESlateVisibility::Hidden);
	// ActiveStateContainer->SetVisibility(ESlateVisibility::Hidden);
}

void UNpcStateWidget::SetNPC(ACharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;

	auto Npc = Cast<INpc>(InOwnerCharacter);
	auto AliveCreature = Cast<INpcAliveCreature>(InOwnerCharacter);
	auto ASCOwner = Cast<IAbilitySystemInterface>(InOwnerCharacter);
	auto ASC = ASCOwner->GetAbilitySystemComponent();
	auto NpcCombatComponent = InOwnerCharacter->FindComponentByClass<UNpcComponent>();
	
	NameTextblock->SetText(Npc->GetNpcName());

	bool bFound;
	auto HealthAttribute = AliveCreature->GetHealthAttribute();
	Health = ASC->GetGameplayAttributeValue(HealthAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute).AddUObject(this, &UNpcStateWidget::OnNpcHealthChanged);

	auto MaxHealthAttribute = AliveCreature->GetMaxHealthAttribute();
	MaxHealth = ASC->GetGameplayAttributeValue(MaxHealthAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttribute).AddUObject(this, &UNpcStateWidget::OnNpcMaxHealthChanged);

	auto AggressionAttribute = UNpcCombatAttributeSet::GetAggressionAttribute();
	Aggression = ASC->GetGameplayAttributeValue(AggressionAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(AggressionAttribute).AddUObject(this, &UNpcStateWidget::OnAggressionChanged);

	auto MaxAggressionAttribute = UNpcCombatAttributeSet::GetMaxAggressionAttribute();
	MaxAggression = ASC->GetGameplayAttributeValue(MaxAggressionAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(MaxAggressionAttribute).AddUObject(this, &UNpcStateWidget::MaxOnAggressionChanged);

	auto PoiseAttribute = AliveCreature->GetPoiseAttribute();
	Poise = ASC->GetGameplayAttributeValue(PoiseAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(PoiseAttribute).AddUObject(this, &UNpcStateWidget::OnPoiseChanged);

	auto MaxPoiseAttribute = AliveCreature->GetMaxPoiseAttribute();
	MaxPoise = ASC->GetGameplayAttributeValue(MaxPoiseAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(MaxPoiseAttribute).AddUObject(this, &UNpcStateWidget::OnMaxPoiseChanged);

	auto StaminaAttribute = AliveCreature->GetStaminaAttribute();
	Stamina = ASC->GetGameplayAttributeValue(StaminaAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(StaminaAttribute).AddUObject(this, &UNpcStateWidget::OnStaminaChanged);

	auto MaxStaminaAttribute = AliveCreature->GetMaxStaminaAttribute();
	MaxStamina = ASC->GetGameplayAttributeValue(MaxStaminaAttribute, bFound);
	if (bFound)
		ASC->GetGameplayAttributeValueChangeDelegate(MaxStaminaAttribute).AddUObject(this, &UNpcStateWidget::OnMaxStaminaChanged);

	SetStatWidgetData(HealthTextBlock, HealthBar, Health, MaxHealth);
	SetStatWidgetData(AggressionTextBlock, AggressionBar, Aggression, MaxAggression);
	SetStatWidgetData(StaminaTextBlock, StaminaBar, Stamina, MaxStamina);
	SetStatWidgetData(PoiseTextBlock, PoiseBar, Poise, MaxPoise);
	
	NpcCombatComponent->OnStateChanged.AddUObject(this, &UNpcStateWidget::OnStateChanged);
	NpcCombatComponent->OnBehaviorChanged.AddUObject(this, &UNpcStateWidget::OnBehaviorChanged);
	NpcCombatComponent->OnActiveAbilityChanged.AddUObject(this, &UNpcStateWidget::OnActiveAbilityChanged);

	UpdateHostileState();
}

void UNpcStateWidget::SetHostile(bool bHostileNew)
{
	if (bHostile == bHostileNew)
		return;

	bHostile = bHostileNew;
	UpdateHostileState();
}

void UNpcStateWidget::SetDetailedNpcView(bool bDetailed)
{
	ESlateVisibility NewBarsVisibility = bDetailed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden;
	StaminaBar->SetVisibility(NewBarsVisibility);
	PoiseBar->SetVisibility(NewBarsVisibility);
	AggressionBar->SetVisibility(NewBarsVisibility);
}

void UNpcStateWidget::OnNpcHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	Health = OnAttributeChangeData.NewValue;
	if (Health <= 0.f)
		SetVisibility(ESlateVisibility::Hidden);
	else
		SetVisibility(ESlateVisibility::HitTestInvisible);
		SetStatWidgetData(HealthTextBlock, HealthBar, Health, MaxHealth);
}

void UNpcStateWidget::OnNpcMaxHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	MaxHealth = OnAttributeChangeData.NewValue;
	SetStatWidgetData(HealthTextBlock, HealthBar, Health, MaxHealth);
}

void UNpcStateWidget::OnAggressionChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	Aggression = OnAttributeChangeData.NewValue;
	SetStatWidgetData(AggressionTextBlock, AggressionBar, Aggression, MaxAggression);
}

void UNpcStateWidget::MaxOnAggressionChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	MaxAggression = OnAttributeChangeData.NewValue;
	SetStatWidgetData(AggressionTextBlock, AggressionBar, Aggression, MaxAggression);
}

void UNpcStateWidget::OnPoiseChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	Poise = OnAttributeChangeData.NewValue;
	SetStatWidgetData(PoiseTextBlock, PoiseBar, Poise, MaxPoise);
}

void UNpcStateWidget::OnMaxPoiseChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	MaxPoise = OnAttributeChangeData.NewValue;
	SetStatWidgetData(PoiseTextBlock, PoiseBar, Poise, MaxPoise);
}

void UNpcStateWidget::OnStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	Stamina = OnAttributeChangeData.NewValue;
	SetStatWidgetData(StaminaTextBlock, StaminaBar, Stamina, MaxStamina);
}

void UNpcStateWidget::OnMaxStaminaChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	MaxStamina = OnAttributeChangeData.NewValue;
	SetStatWidgetData(StaminaTextBlock, StaminaBar, Stamina, MaxStamina);
}

void UNpcStateWidget::SetStatWidgetData(UTextBlock* TextBlock, UProgressBar* ProgressBar, float Value, float MaxValue)
{
	ProgressBar->SetPercent(Value / MaxValue);
	TextBlock->SetText(FText::FromString(FString::Printf(TEXT("%.2f / %.2f"), Value, MaxValue)));
}

void UNpcStateWidget::OnStateChanged(const FGameplayTag& StateTag, bool bActive)
{
	// UpdateStateWidgets(ActiveStateContainer, StateTag, bActive);
}

void UNpcStateWidget::OnBehaviorChanged(const FGameplayTag& BehaviorTag, bool bActive)
{
	// UpdateStateWidgets(ActiveBehaviorContainer, BehaviorTag, bActive);
}

void UNpcStateWidget::OnActiveAbilityChanged(const FGameplayTag& AbilityTag, bool bActive)
{
	// UpdateStateWidgets(ActiveAbilityContainer, AbilityTag, bActive);
}

void UNpcStateWidget::UpdateStateWidgets(UNpcStateCategoryWidget* ContainerWidget, const FGameplayTag& Tag, bool bActive)
{
	ContainerWidget->OnItemChanged(Tag, bActive);
}

void UNpcStateWidget::UpdateHostileState()
{
	BarsSizeBox->SetVisibility(bHostile ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
