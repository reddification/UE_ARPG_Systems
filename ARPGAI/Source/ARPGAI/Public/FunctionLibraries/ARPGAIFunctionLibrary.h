// 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ARPGAIFunctionLibrary.generated.h"

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class ARPGAI_API UARPGAIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsActorEnemyForNpc(AActor* Npc, AActor* TestedActor);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FGameplayTag GetNpcAttitude(AActor* Npc, AActor* TestedActor);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsActorAlive(AActor* TestActor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
    static FGameplayTag RequestGameplayTagFromName(const FName& TagName, bool bErrorOnFail);
};
