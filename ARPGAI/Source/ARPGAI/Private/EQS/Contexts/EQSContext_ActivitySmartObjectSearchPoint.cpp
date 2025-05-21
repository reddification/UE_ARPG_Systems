// 


#include "EQS/Contexts/EQSContext_ActivitySmartObjectSearchPoint.h"

#include "Components/Controller/NpcActivityComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "Gameframework/GameModeBase.h"
#include "Interfaces/NpcSystemGameMode.h"

void UEQSContext_ActivitySmartObjectSearchPoint::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                                FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn) || !QuerierPawn->Controller)
	{
		return;
	}

	auto NpcActivityComponent = QuerierPawn->Controller->FindComponentByClass<UNpcActivityComponent>();
	if (!NpcActivityComponent)
		return;

	auto ActiveGoal = Cast<UNpcGoalUseSmartObject>(NpcActivityComponent->GetActiveGoal());
	FVector Result = NpcActivityComponent->GetPawnLocation();
	if (ensure(ActiveGoal))
	{
		FNpcGoalParameters_UseSmartObject Parameters = ActiveGoal->GetParameters(QuerierPawn);
		if (Parameters.LocationIdTag.IsValid())
		{
			auto NpcGameMode = Cast<INpcSystemGameMode>(QuerierPawn->GetWorld()->GetAuthGameMode());
			if (ensure(NpcGameMode))
			{
				Result = NpcGameMode->GetNpcLocation(Parameters.LocationIdTag, false);
			}
		}
	}

	UEnvQueryItemType_Point::SetContextHelper(ContextData, Result);
}
