// 


#include "Objects/ArbitraryQuestAction.h"

void UArbitraryQuestAction::Execute_Implementation(const FArbitraryQuestActionContext& Context,
                                                   const TMap<FGameplayTag, FGameplayTag>& TagParameters, const TMap<FGameplayTag, float>& FloatParameters)
{
	
}

UWorld* UArbitraryQuestAction::GetWorld() const
{
	const AActor* Outer = Cast<AActor>(GetOuter());
	return Outer ? Outer->GetWorld() : nullptr;
}