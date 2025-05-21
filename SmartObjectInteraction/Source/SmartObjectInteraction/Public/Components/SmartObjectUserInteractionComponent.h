// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayBehaviors/GameplayBehaviorConfig_Interaction.h"
#include "SmartObjectRuntime.h"
#include "SmartObjectUserInteractionComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SMARTOBJECTINTERACTION_API USmartObjectUserInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USmartObjectUserInteractionComponent();
	
	virtual bool StartUseSmartObject(const FGameplayTag& GestureTag, AActor* SmartObjectOwner, const FGameplayTagContainer& GrantedTags, bool bSmartObjectUserTagsPermanent);
	virtual void StopUseSmartObject();

	void ReportInteractableSmartObjectEvent(const FGameplayTag& InteractionEventTag);
	
	FORCEINLINE const FSmartObjectClaimHandle& GetActiveSOCH() { return ActiveSOCH; }
	FORCEINLINE void SetActiveSOCH(const FSmartObjectClaimHandle& InSOCH) { ActiveSOCH = InSOCH; };
	FORCEINLINE bool IsInteracting() const { return ActiveSmartObjectOwner.IsValid(); };
	FORCEINLINE AActor* GetActiveSmartObjectActor() const { return ActiveSmartObjectOwner.Get(); }

private:
	TWeakObjectPtr<AActor> ActiveSmartObjectOwner;
	FGameplayTagContainer GrantedTags;
	FSmartObjectClaimHandle ActiveSOCH;
	bool bSmartObjectGrantedUserTagsPermanent = false;
};
