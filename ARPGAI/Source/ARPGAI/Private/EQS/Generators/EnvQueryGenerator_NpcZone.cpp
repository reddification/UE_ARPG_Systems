#include "EQS/Generators/EnvQueryGenerator_NpcZone.h"

#include "EQS/Contexts/EnvQueryContext_RoamingZone.h"
#include "Interfaces/NpcZone.h"

UEnvQueryGenerator_NpcZone::UEnvQueryGenerator_NpcZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GenerateAround = UEnvQueryContext_RoamingZone::StaticClass();
	ExtentScale.DefaultValue = 1.25f;
	PointsDensity.DefaultValue = 1.f;
}

void UEnvQueryGenerator_NpcZone::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	const UObject* BindOwner = QueryInstance.Owner.Get();
	ExtentScale.BindData(BindOwner, QueryInstance.QueryID);
	PointsDensity.BindData(BindOwner, QueryInstance.QueryID);

	TArray<AActor*> RoamingZones;
	QueryInstance.PrepareContext(GenerateAround, RoamingZones);
	if (RoamingZones.Num() <= 0)
		return;

	TArray<FNavLocation> GridPoints;
	for (const auto& RoamingZone : RoamingZones)
	{
		auto NpcZone = Cast<INpcZone>(RoamingZone);
		GridPoints.Append(NpcZone->ProvideEqsPoints(PointsDensity.GetValue(), ExtentScale.GetValue()));
	}
	
	ProjectAndFilterNavPoints(GridPoints, QueryInstance);
	StoreNavPoints(GridPoints, QueryInstance);
}

FText UEnvQueryGenerator_NpcZone::GetDescriptionTitle() const
{
	return FText::FromString("Generate items within Roaming zone");
}