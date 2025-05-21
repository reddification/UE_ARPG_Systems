#pragma once

struct FCombatCollisionShapeData;

TArray<FCombatCollisionShapeData> GetCombatCollisionShapes(const FName& ExpectedCollisionShapeName, const TArray<USkeletalMeshComponent*>& SkeletalMeshComponents, const AActor*
                                                           LogOwner);
