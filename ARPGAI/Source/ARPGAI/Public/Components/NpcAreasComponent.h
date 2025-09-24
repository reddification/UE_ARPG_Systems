// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Data/AiDataTypes.h"
#include "NpcAreasComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcAreasComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcAreasComponent();
	
	void AddAreaOfInterest(const FGameplayTag& AreaType, TScriptInterface<INpcZone> NewArea);
	void AddAreasOfInterest(const FGameplayTag& AreaType, const FGameplayTagContainer& AreaIds);
	void RemoveNpcAreas(const FGameplayTag& AreaType);
	void RemoveNpcArea(const FGameplayTag& AreaType, TScriptInterface<INpcZone> AreaToRemove);
	bool SetExclusiveNpcAreaType(const FGameplayTag& NewExclusiveAreaType);
	bool RemoveExclusiveNpcAreaType(const FGameplayTag& RemovedExclusiveAreaType);
	const TMap<FGameplayTag, FNpcAreasContainer>& GetNpcAreas() const;
	bool IsLocationWithinNpcArea(const FVector& TestLocation, float AreaExtent) const;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer AreasOfInterestTags;

	FGameplayTag CurrentExclusiveAreaType = FGameplayTag::EmptyTag;

	// key tag - what an area means to the NPC. examples/suggestions tags: spawn zone, activity, protection, restriction 
	UPROPERTY()
	TMap<FGameplayTag, FNpcAreasContainer> NpcAreas;

	// This one is used when there's a limit to only one area type (for some specific quest activity, for example)
	UPROPERTY()
	TMap<FGameplayTag, FNpcAreasContainer> ExclusiveNpcAreas;
};
