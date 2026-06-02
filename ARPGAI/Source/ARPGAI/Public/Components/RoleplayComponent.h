// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "RoleplayComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API URoleplayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URoleplayComponent();
	
	FORCEINLINE const FGameplayTag& GetObjective() const { return CurrentObjectiveTag; }
	
	virtual void SetPersonality(const FGameplayTag& Personality);
	virtual void SetTemper(const FGameplayTag& NewTemperTag);
	virtual void SetObjective(const FGameplayTag& NewObjectiveTag);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag IdentityPersonalityTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag IdentityTemperTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CurrentObjectiveTag;
};
