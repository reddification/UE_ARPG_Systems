// 


#include "EQS/Generators/EnvQueryGenerator_RoamingZone.h"

#include "Components/NpcSpawnerComponent.h"
#include "EQS/Contexts/EnvQueryContext_RoamingZone.h"

UEnvQueryGenerator_RoamingZone::UEnvQueryGenerator_RoamingZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GenerateAround = UEnvQueryContext_RoamingZone::StaticClass();
	ExtentScale.DefaultValue = 1.25f;
	SpaceBetween.DefaultValue = 100.0f;
}

void UEnvQueryGenerator_RoamingZone::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	const UObject* BindOwner = QueryInstance.Owner.Get();
	ExtentScale.BindData(BindOwner, QueryInstance.QueryID);
	SpaceBetween.BindData(BindOwner, QueryInstance.QueryID);

	TArray<AActor*> RoamingZones;
	QueryInstance.PrepareContext(GenerateAround, RoamingZones);
	if (RoamingZones.Num() != 1)
	{
		return;
	}

	const UNpcSpawnerComponent* RoamingZoneComponent =RoamingZones[0]->FindComponentByClass<UNpcSpawnerComponent>();
	TArray<FNavLocation> GridPoints;
	RoamingZoneComponent->ProvideEqsPoints(GridPoints, SpaceBetween.GetValue(), ExtentScale.GetValue());
	ProjectAndFilterNavPoints(GridPoints, QueryInstance);
	StoreNavPoints(GridPoints, QueryInstance);
}

FText UEnvQueryGenerator_RoamingZone::GetDescriptionTitle() const
{
	return FText::FromString("Generate items within Roaming zone");
}