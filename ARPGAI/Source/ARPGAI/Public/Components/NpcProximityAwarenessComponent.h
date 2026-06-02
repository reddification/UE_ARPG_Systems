#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NpcProximityAwarenessComponent.generated.h"


struct FProximityAwarenessData
{
	FProximityAwarenessData() { Duration = 0.f; }
	float Duration = 0.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcProximityAwarenessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcProximityAwarenessComponent();
	const TMap<TWeakObjectPtr<AActor>, FProximityAwarenessData>& GetDetectedActorsInProximity() const { return ActorsInProximity; }
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void ActivateProximityAwareness(float Radius, float UpdateInterval, const TArray<TEnumAsByte<ECollisionChannel>>& ObjectTypes,
	                                bool bIgnoreAllies = true, const FGameplayTagQuery* OptionalDetectionBlockedFIlter = nullptr);
	void DisableProximityAwareness();
	
protected:
	virtual void BeginPlay() override;

private:
	bool CanDetect(AActor* Actor) const;
	void OnActorEnterProximity(AActor* OtherActor);
	void OnActorExitProximity(AActor* OtherActor);
	void OnAsyncOverlapCompleted(const FTraceHandle& TraceHandle, FOverlapDatum& OverlapDatum);
	
	FCollisionObjectQueryParams CollisionObjectQueryParams;
	FCollisionQueryParams CollisionQueryParams;
	TMap<TWeakObjectPtr<AActor>, FProximityAwarenessData> ActorsInProximity;
	FGameplayTagQuery DetectionBlockedTagQuery;
	FOverlapDelegate OverlapDelegate;
	float OverlapRadius = 300.f;
	bool bIgnoreAllies = true;
};
