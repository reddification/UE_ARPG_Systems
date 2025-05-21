#pragma once
#include "Engine/DataTable.h"
#include "WorldLocationDTR.generated.h"

USTRUCT(BlueprintType)
struct FWorldLocationDTR : public FTableRowBase
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
	bool bQuestLocation = false;
};