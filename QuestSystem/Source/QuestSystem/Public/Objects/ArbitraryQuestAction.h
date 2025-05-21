// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ArbitraryQuestAction.generated.h"

struct FGameplayTag;

USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FArbitraryQuestActionContext
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	APawn* PlayerCharacter;
};

UCLASS(Blueprintable)
class QUESTSYSTEM_API UArbitraryQuestAction : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Execute(const FArbitraryQuestActionContext& QuestActionContext, const TMap<FGameplayTag, FGameplayTag>& TagParameters, const TMap<FGameplayTag, float>& FloatParameters);

	virtual UWorld* GetWorld() const override;
};
