#pragma once

#include "CoreMinimal.h"
#include "GameplayBehavior.h"
#include "GameplayBehavior_Interaction.generated.h"

UCLASS()
class SMARTOBJECTINTERACTION_API UGameplayBehavior_Interaction : public UGameplayBehavior
{
	GENERATED_BODY()

public:
	UGameplayBehavior_Interaction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool Trigger(AActor& Avatar, const UGameplayBehaviorConfig* Config, AActor* SmartObjectOwner) override;
	virtual void EndBehavior(AActor& Avatar, const bool bInterrupted) override;
};
