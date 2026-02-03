// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SphereComponent.h"
#include "NpcProximityAwarenessComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcProximityAwarenessComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UNpcProximityAwarenessComponent();
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	const TArray<TWeakObjectPtr<AActor>>& GetDetectedActorsInProximity() const { return ActorsInProximity; }
	
protected:
	virtual void BeginPlay() override;

	// owner won't detect actor with these tags
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery DetectionBlockedTagQuery;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AwarenessCollisionProfileName = FName("NpcAwareness");
	
	TArray<TWeakObjectPtr<AActor>> ActorsInProximity;

private:
	bool CanDetect(AActor* Actor) const;
	
	UFUNCTION()
	void OnActorEnterProximity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
							   const FHitResult& SweepResult);
	UFUNCTION()
	void OnActorExitProximity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
