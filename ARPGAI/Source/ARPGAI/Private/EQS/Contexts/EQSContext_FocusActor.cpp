


#include "EQS/Contexts/EQSContext_FocusActor.h"

#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEQSContext_FocusActor::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                 FEnvQueryContextData& ContextData) const
{
	const APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!IsValid(QuerierPawn))
	{
		return;
	}

	const AActor* ResultingActor = nullptr;
	const AAIController* AIController = Cast<AAIController>(QuerierPawn->GetController());
	if (IsValid(AIController))
	{
		ResultingActor = AIController->GetFocusActor();
	}
	
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, ResultingActor);
}
