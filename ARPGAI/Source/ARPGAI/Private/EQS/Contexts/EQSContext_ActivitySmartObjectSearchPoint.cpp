// 


#include "EQS/Contexts/EQSContext_ActivitySmartObjectSearchPoint.h"

#include "Data/AIGameplayTags.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Gameframework/GameModeBase.h"
#include "Interfaces/NpcGoalManager.h"
#include "Interfaces/NpcSystemGameMode.h"

void UEQSContext_ActivitySmartObjectSearchPoint::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                                FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
		return;

	auto NpcGoalManager = Cast<INpcGoalManager>(QuerierPawn->Controller);
	if (!NpcGoalManager)
		return;

	FGameplayTag LocationId = NpcGoalManager->GetGoalTagParameter(AIGameplayTags::Activity_Goal_Parameter_LocationId);
	FVector Result = QuerierPawn->GetActorLocation();
	if (LocationId.IsValid()) // can be empty. in this case - use querier location
	{
		auto NpcGameMode = Cast<INpcSystemGameMode>(QuerierPawn->GetWorld()->GetAuthGameMode());
		if (ensure(NpcGameMode))
			Result = NpcGameMode->GetNpcLocation(LocationId, QuerierPawn->GetActorLocation(), false);
	}

	UEnvQueryItemType_Point::SetContextHelper(ContextData, Result);
}
