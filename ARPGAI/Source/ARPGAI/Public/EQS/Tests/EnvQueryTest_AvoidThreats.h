

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_AvoidThreats.generated.h"

UCLASS()
class ARPGAI_API UEnvQueryTest_AvoidThreats : public UEnvQueryTest
{
	GENERATED_BODY()

private:
	struct FAvoidThreatData
	{
		FVector Location = FVector::ZeroVector;
		const AActor* Actor = nullptr;
		float Score = 1.f;
	};
	
public:
	UEnvQueryTest_AvoidThreats();

protected:
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionDetails() const override;

	UPROPERTY(EditDefaultsOnly, Category=Filter)
	FAIDataProviderFloatValue ThreatSpeedThresholdValue;

	UPROPERTY(EditDefaultsOnly, Category=Filter)
	FAIDataProviderFloatValue ThreatPredictionTimeValue;

	UPROPERTY(EditDefaultsOnly, Category=Filter)
	FAIDataProviderFloatValue GeneratorRadiusValue;
};
