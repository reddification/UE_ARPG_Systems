#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "SmartObjectInteractionChecker.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct SMARTOBJECTINTERACTION_API FSmartObjectInteractionChecker
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag RefuseTag = FGameplayTag::EmptyTag;
	
	virtual ~FSmartObjectInteractionChecker() = default;
	virtual bool CanStartInteractWithSmartObject(AActor* Instigator, FGameplayTag& RefuseReason)
	{
		RefuseReason = FGameplayTag::EmptyTag;
		return true;
	}
	virtual void HandleStartSmartInteraction(AActor* Instigator) { return; }
	virtual void HandleEndSmartInteraction(AActor* Instigator) { return; }
};
