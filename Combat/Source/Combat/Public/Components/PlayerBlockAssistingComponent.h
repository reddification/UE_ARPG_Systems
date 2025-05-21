// 

#pragma once

#include "CoreMinimal.h"
#include "Components/MeleeBlockComponent.h"
#include "PlayerBlockAssistingComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COMBAT_API UPlayerBlockAssistingComponent : public UMeleeBlockComponent
{
	GENERATED_BODY()

public:
	virtual void StartBlocking() override;

protected:
	virtual FVector2D GetBlockInput(float DeltaTime) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AutoTargetSearchRange = 300.f;

private:
	TScriptInterface<ICombatant> FindAutoBlockTarget(AActor* OwnerLocal);

	FVector2D BlockDirection = FVector2D::ZeroVector;
};
