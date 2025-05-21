

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NpcPerceptionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcPerceptionComponent : public UAIPerceptionComponent
{
	GENERATED_BODY()

	struct FPerceptionMemoryData
	{
		float TimeSeen = 0.f;
	};

public:
	UNpcPerceptionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float GetAccumulatedTimeSeen(AActor* Actor) const;

	UFUNCTION(BlueprintCallable)
	float GetAccumulatedDamage() const;
	
protected:
	virtual void BeginPlay() override;
	virtual void RefreshStimulus(FAIStimulus& StimulusStore, const FAIStimulus& NewStimulus) override;

private:
	UFUNCTION()
	void OnTargetPerceptionInfoUpdatedHandler(const FActorPerceptionUpdateInfo& UpdateInfo);

	UFUNCTION()
	void OnTargetPerceptionForgottenHandler(AActor* Actor);
	
	TMap<TWeakObjectPtr<AActor>, FPerceptionMemoryData> ActorsMemory;
};
