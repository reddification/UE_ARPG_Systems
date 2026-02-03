// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "NpcQueueComponent.generated.h"

class USplineComponent;

USTRUCT(BlueprintType)
struct ARPGAI_API FNpcQueueMemberPosition
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector QueuePointLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator QueuePointRotation = FRotator::ZeroRotator;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int PlaceInQueue = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bEntered = false;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FNpcQueueMemberAdvancedEvent, AActor* Member, const FNpcQueueMemberPosition& NpcQueueMemberPosition)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcQueueComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	struct FNpcQueuePoint
	{
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		TWeakObjectPtr<AActor> OccupiedBy;
	};
	
public:
	UNpcQueueComponent();

	UFUNCTION(BlueprintCallable)
	FNpcQueueMemberPosition StandInQueue(AActor* NewQueueMember);

	UFUNCTION(BlueprintCallable)
	FNpcQueueMemberPosition StandInQueueAtPosition(AActor* NewQueueMember, int DesiredQueuePosition);
	
	FNpcQueueMemberPosition GetNpcQueuePosition(const APawn* Pawn) const;

	UFUNCTION(BlueprintCallable)
	void LeaveQueue(AActor* LeftQueueMember);
	
	FORCEINLINE bool IsFull() const { return LastQueuePlaceIndex >= QueuePoints.Num(); }

	FNpcQueueMemberAdvancedEvent NpcQueueMemberAdvancedEvent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer CustomQueueTags;

protected:
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	FNpcQueueMemberPosition StandInQueue(AActor* NewQueueMember, int StandAtPosition);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag QueueId;

private:
	TWeakObjectPtr<USplineComponent> OwnerSplineComponent;
	TArray<FNpcQueuePoint> QueuePoints;
	int LastQueuePlaceIndex = -1;
};
