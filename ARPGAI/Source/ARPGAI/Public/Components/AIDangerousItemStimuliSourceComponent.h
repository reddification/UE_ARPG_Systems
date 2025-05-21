

#pragma once

#include "CoreMinimal.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "AIDangerousItemStimuliSourceComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UAIDangerousItemStimuliSourceComponent : public UAIPerceptionStimuliSourceComponent
{
	GENERATED_BODY()

public:
	UAIDangerousItemStimuliSourceComponent(const FObjectInitializer& ObjectInitializer);
	float GetDangerScore() const;

protected:
	virtual void BeginPlay() override;

private:
	float DangerScore = 1.f;
};
