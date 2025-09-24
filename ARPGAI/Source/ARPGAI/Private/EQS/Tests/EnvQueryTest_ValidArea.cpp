// 


#include "EQS/Tests/EnvQueryTest_ValidArea.h"

#include "Components/NpcAreasComponent.h"
#include "Components/NpcComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Interfaces/NpcZone.h"

UEnvQueryTest_ValidArea::UEnvQueryTest_ValidArea()
{
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Filter;
	SetWorkOnFloatValues(false);
}

void UEnvQueryTest_ValidArea::RunTest(FEnvQueryInstance& QueryInstance) const
{
	auto OwnerPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (OwnerPawn == nullptr)
		return;

	auto NpcComponent = OwnerPawn->FindComponentByClass<UNpcAreasComponent>();
	const auto& NpcAreas = NpcComponent->GetNpcAreas();
	if (NpcAreas.Num() == 0)
	{
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			It.SetScore(TestPurpose, FilterType, true, true);
	}
	else
	{
		AreaExtent.BindData(OwnerPawn, QueryInstance.QueryID);
		const float AreaExtentValue = AreaExtent.GetValue();
		
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			bool bLocationWithinNpcAreas = false;
			const auto ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			
			for (const auto& NpcAreaType : NpcAreas)
			{
				for (const auto& NpcArea : NpcAreaType.Value.NpcAreas)
				{
					if (NpcArea->IsLocationWithinNpcArea(ItemLocation, AreaExtentValue))
					{
						bLocationWithinNpcAreas = true;
						break;
					}
				}

				if (bLocationWithinNpcAreas)
					break;
			}
			
			It.SetScore(TestPurpose, FilterType, bLocationWithinNpcAreas, true);
		}
	}
}
