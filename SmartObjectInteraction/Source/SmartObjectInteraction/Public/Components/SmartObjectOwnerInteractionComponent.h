// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "SmartObjectOwnerInteractionComponent.generated.h"

struct FGameplayTagContainer;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SMARTOBJECTINTERACTION_API USmartObjectOwnerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USmartObjectOwnerInteractionComponent();
	
	virtual void OnInteractionStarted(const FGameplayTagContainer& InSmartObjectTags, bool bSmartObjectActorTagsPermanent);
	virtual void OnInteractionEnded();

	UPROPERTY(EditDefaultsOnly, meta = (ExcludeBaseStruct))
	TInstancedStruct<struct FSmartObjectInteractionChecker> InteractionChecker;

private:
	FGameplayTagContainer GrantedTags;
	bool bSmartObjectGrantedTagsPermanent = false;
};