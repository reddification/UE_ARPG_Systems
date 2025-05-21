#include "CombatCommonHelpers.h"

#include "Data/CombatDataTypes.h"
#include "Data/CombatLogChannels.h"
#include "PhysicsEngine/PhysicsAsset.h"

TArray<FCombatCollisionShapeData> GetCombatCollisionShapes(const FName& ExpectedCollisionShapeName,
                                                           const TArray<USkeletalMeshComponent*>& SkeletalMeshComponents, const AActor* LogOwner)
{
	TArray<FCombatCollisionShapeData> Result;
	bool bHasSkeletalMesh = false;

	TRACE_CPUPROFILER_EVENT_SCOPE(GetCombatCollisionShapes)
	
	for (auto WeaponSkeletalMesh : SkeletalMeshComponents)
	{
		if (WeaponSkeletalMesh->GetSkeletalMeshAsset() == nullptr)
		{
			if (LogOwner)
			{
				UE_VLOG(LogOwner, LogCombat, Error, TEXT("GetCombatCollisionShapes: No skeletal mesh asset"));
			}
	
			continue;
		}
		
		auto PhysicsAsset = WeaponSkeletalMesh->GetPhysicsAsset();
		if (PhysicsAsset == nullptr)
		{
			if (LogOwner)
			{
				UE_VLOG(LogOwner, LogCombat, Error, TEXT("No physics asset for %s"), *WeaponSkeletalMesh->GetSkeletalMeshAsset()->GetName());
			}
			
			continue;
		}
		
		for (auto CollisionBodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			if (!CollisionBodySetup)
			{
				if (LogOwner)
				{
					UE_VLOG(LogOwner, LogCombat, Error, TEXT("No skeletal body setup found asset for %s"), *WeaponSkeletalMesh->GetSkeletalMeshAsset()->GetName());
				}
				
				continue;
			}

			if (CollisionBodySetup->AggGeom.BoxElems.Num() == 1)
			{
				const auto& BoxElem = &CollisionBodySetup->AggGeom.BoxElems[0];
				FCombatCollisionShapeData WeaponCollisionShapeData;
				WeaponCollisionShapeData.Type = FCombatCollisionShapeData::Box;
				WeaponCollisionShapeData.Center = BoxElem->Center;
				WeaponCollisionShapeData.Extent = FVector(BoxElem->X, BoxElem->Y, BoxElem->Z);
				WeaponCollisionShapeData.HalfHeight = BoxElem->Z * 0.5f;
				WeaponCollisionShapeData.Radius = FMath::Sqrt(BoxElem->X * BoxElem->X + BoxElem->Y * BoxElem->Y);
				WeaponCollisionShapeData.Rotation = BoxElem->Rotation;
				Result.Add(WeaponCollisionShapeData);
			}
			else
			{
				for (const auto& BoxCollisionShape : CollisionBodySetup->AggGeom.BoxElems)
				{
					if (BoxCollisionShape.GetName() == ExpectedCollisionShapeName)
					{
						FCombatCollisionShapeData WeaponCollisionShapeData;
						WeaponCollisionShapeData.Type = FCombatCollisionShapeData::Box;
						WeaponCollisionShapeData.Center = BoxCollisionShape.Center;
						WeaponCollisionShapeData.Extent = FVector(BoxCollisionShape.X, BoxCollisionShape.Y, BoxCollisionShape.Z);
						WeaponCollisionShapeData.HalfHeight = BoxCollisionShape.Z * 0.5f;
						WeaponCollisionShapeData.Radius = FMath::Sqrt(BoxCollisionShape.X * BoxCollisionShape.X + BoxCollisionShape.Y * BoxCollisionShape.Y);
						WeaponCollisionShapeData.Rotation = BoxCollisionShape.Rotation;
						Result.Add(WeaponCollisionShapeData);		
					}
				}	
			}

			if (Result.Num() == 0)
			{
				ensure(false);
				if (LogOwner)
				{
					UE_VLOG(LogOwner, LogCombat, Error, TEXT("No box collisions found for %s"), *WeaponSkeletalMesh->GetSkeletalMeshAsset()->GetName());
				}
			}

			// @AK 25.04.2025 Currently I think we should abandoned any non-box collision for weapons, because they tend to roll on ground instead of staying 
			// for (const auto& CapsuleCollisionShape : CollisionBodySetup->AggGeom.SphylElems)
			// {
			// 	if (CapsuleCollisionShape.GetName() == ExpectedCollisionShapeName)
			// 	{
			// 		FCombatCollisionShapeData WeaponCollisionShapeData;
			// 		WeaponCollisionShapeData.Type = FCombatCollisionShapeData::Cylinder;
			// 		WeaponCollisionShapeData.Center = CapsuleCollisionShape.Center;
			// 		WeaponCollisionShapeData.Radius = CapsuleCollisionShape.Radius;
			// 		WeaponCollisionShapeData.HalfHeight = CapsuleCollisionShape.Length / 2.f;
			// 		WeaponCollisionShapeData.Rotation = CapsuleCollisionShape.Rotation;
			// 		Result.Add(WeaponCollisionShapeData);		
			// 	}
			// }
			//
			// for (const auto& ConvexCollisionShape : CollisionBodySetup->AggGeom.ConvexElems)
			// {
			// 	if (ConvexCollisionShape.GetName() == ExpectedCollisionShapeName)
			// 	{
			// 		FCombatCollisionShapeData WeaponCollisionShapeData;
			// 		WeaponCollisionShapeData.Type = FCombatCollisionShapeData::Convex;
			// 		Result.Add(WeaponCollisionShapeData);
			// 		ensure(false); // TODO implement logic for it
			// 	}
			// }
		}
	}
	
	return Result;
}
