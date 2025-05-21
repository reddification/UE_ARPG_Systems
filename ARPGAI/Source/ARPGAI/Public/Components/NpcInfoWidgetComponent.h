// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "Widgets/NpcStateWidget.h"
#include "NpcInfoWidgetComponent.generated.h"


class UNpcComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcInfoWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNpcInfoWidgetComponent();

	// virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDisplayDetailedNpcView = false;
	
private:
	FTimerHandle UpdateVisibilityTimer;
	void UpdateVisibility();
	// bool bScaleWidget = false;

	void InitializeNpc();
	
	TWeakObjectPtr<class APawn> PlayerPawn;
	TWeakObjectPtr<UNpcStateWidget> NpcStateWidget;
	TWeakObjectPtr<UNpcComponent> OwnerNpcComponent;

	// TSoftObjectPtr<UCurveFloat> WidgetScaleByDistanceToPlayerDependencyPtr; 
	
	// UPROPERTY()
	// UCurveFloat* WidgetScaleByDistanceToPlayerDependency = nullptr;

	float MinDotProductToShowWidget = -0.5f;
	float ConsiderableDistanceToPlayerForHostile = 3000.f;
	float ConsiderableDistanceToPlayerForNonHostile = 400.f;
	
	UFUNCTION()
	void OnDeathStarted(AActor* OwningActor);
};
