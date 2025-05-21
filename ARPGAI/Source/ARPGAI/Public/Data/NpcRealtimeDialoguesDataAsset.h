// 

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RealtimeDialogueDataTypes.h"
#include "Engine/DataAsset.h"
#include "NpcRealtimeDialoguesDataAsset.generated.h"


struct FGameplayTagContainer;

USTRUCT(BlueprintType)
struct FDialogueRefuseReason
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery NpcTagsQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery RequestorTagsQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagQuery WorldStateQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bRequireAllFilterPass = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag RefuseReason;
};

UCLASS()
class ARPGAI_API UNpcPhrasesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FNpcRealtimeDialogueLines> NpcPhrases;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Dialogue")
	TArray<FDialogueRefuseReason> DialogueRefuseReasons;

};
