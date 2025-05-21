// 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetLockComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CheckTargetPriority(const FVector& OwnerEyesLocation, const FVector& ViewDirection, float& BestDP, float& BestTargetDistanceSq,
	                         AActor*& BestTarget, AActor* TestTarget, const FVector& SeenAtLocation);

	AActor* FindTarget();
	void ClearTarget();
	
	FORCEINLINE AActor* GetActiveTarget() const { return Target.Get(); };
	FORCEINLINE bool HasTarget() const { return Target.IsValid(); };

	FSimpleDelegate OnTargetLost;

protected:
	// In context G2VS2 project, this property should match  
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxDistance = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DotProductThreshold = 0.1f;
	
private:
	void OnTrackedCreatureDead(AActor* Actor);

	TWeakObjectPtr<AActor> Target;
	FDelegateHandle CombatCreatureDeadEventDelegateHandle;

};
