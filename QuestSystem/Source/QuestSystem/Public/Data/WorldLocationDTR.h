#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "WorldLocationDTR.generated.h"

class UWorldLocationComponent;

USTRUCT()
struct QUESTSYSTEM_API FWorldLocationCrossedHandler
{
	GENERATED_BODY()

public:
	virtual ~FWorldLocationCrossedHandler() = default;
	virtual void OnLocationCrossed(UWorldLocationComponent* LocationComponent, AActor* CrossedActor, bool bEntered) const {};
};

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FWorldLocationDTR : public FTableRowBase
{
	GENERATED_BODY()

	// Conventience attribute just for the ease of naming the RowName just as Location Id Tag 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Location.Id,G2VS2.Location.Id"))
	FGameplayTag LocationIdTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer LocationTags;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bQuestLocation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FWorldLocationCrossedHandler>> LocationCrossedHandlers;
};