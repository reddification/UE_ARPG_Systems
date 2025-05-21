

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NpcWidgetComponent.generated.h"

UCLASS(ClassGroup=(GladiusMob), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void InitWidget() override;
	FSimpleDelegate OnWidgetComponentInitialized;
};
