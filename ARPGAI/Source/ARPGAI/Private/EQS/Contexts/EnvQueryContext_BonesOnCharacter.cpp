// 


#include "EQS/Contexts/EnvQueryContext_BonesOnCharacter.h"

#include "Activities/NpcComponentsHelpers.h"
#include "Components/NpcComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "GameFramework/Character.h"

void UEnvQueryContext_BonesOnCharacter::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                              FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	if (BonesNames.IsEmpty())
		return;
	
	auto OwnerPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (!OwnerPawn) return;
	
	auto NpcComponent = GetNpcComponent(OwnerPawn);
	if (!NpcComponent) return;
	
	auto FocusedCharacter = Cast<ACharacter>(NpcComponent->GetStoredActor(CharacterStoredDataKey));
	if (FocusedCharacter == nullptr) return;
	
	auto Mesh = FocusedCharacter->GetMesh();
	if (Mesh == nullptr) return;
	
	TArray<FVector> BonesLocations;
	BonesLocations.Reserve(BonesNames.Num());
	for (const auto& BoneName : BonesNames)
	{
		FVector BoneLocation = Mesh->GetBoneLocation(BoneName);
		if (BoneLocation != FVector::ZeroVector)
			BonesLocations.Add(BoneLocation);
	}
	
	UEnvQueryItemType_Point::SetContextHelper(ContextData, BonesLocations);
}
