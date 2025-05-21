


#include "EQS/Tests/EnvQueryTest_AttackCoordinator.h"

#include "Components/EnemiesCoordinatorComponent.h"
#include "EQS/Contexts/EQSContext_FocusActor.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "GameFramework/Character.h"
#include "Settings/NpcCombatSettings.h"

UEnvQueryTest_AttackCoordinator::UEnvQueryTest_AttackCoordinator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestPurpose = EEnvTestPurpose::Type::Score;
	// SetWorkOnFloatValues(true);
}

void UEnvQueryTest_AttackCoordinator::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}

	ACharacter* MobCharacter = Cast<ACharacter>(QueryOwner);
	if (IsValid(MobCharacter) == false)
	{
		return;
	}

	TArray<AActor*> Actors;
	if (!QueryInstance.PrepareContext(BotTargetContext, Actors))
	{
		return;
	}

	if (Actors.Num() != 1)
	{
		return;
	}
	
	const ACharacter* TargetPlayerCharacter = Cast<ACharacter>(Actors[0]);
	if (TargetPlayerCharacter == nullptr)
	{
		return;
	}
	
	const UEnemiesCoordinatorComponent* CoordinatorComponent = TargetPlayerCharacter->FindComponentByClass<UEnemiesCoordinatorComponent>();
	if (CoordinatorComponent == nullptr)
	{
		return;
	}

	float SegmentAngle = 0.f;
	if (CoordinatorComponent->GetEnemyDesiredPosition(MobCharacter, SegmentAngle) == false)
	{
		return;
	}

	const float SurroundDeltaAngle = GetDefault<UNpcCombatSettings>()->SurroundDeltaAngle;
	const float SurroundDotProduct = FMath::Cos(FMath::DegreesToRadians(SurroundDeltaAngle));

	const FVector TargetLocation = TargetPlayerCharacter->GetActorLocation();
	const FVector TargetSegmentVector = TargetPlayerCharacter->GetActorForwardVector().RotateAngleAxis(SegmentAngle, TargetPlayerCharacter->GetActorUpVector());

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
		const float TargetSegmentToItemDirectionDotProduct = FVector::DotProduct((ItemLocation - TargetLocation).GetSafeNormal(), TargetSegmentVector);
		FVector2D SourceRange(SurroundDotProduct, 1.f), TargetRange(0, 1.f);
		const float Result = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, TargetSegmentToItemDirectionDotProduct);
		It.SetScore(TestPurpose, FilterType, 1 + Result, -FLT_MAX, FLT_MAX);
	}
}

FText UEnvQueryTest_AttackCoordinator::GetDescriptionDetails() const
{
	return FText::FromString(TEXT("Scores query items for an attacker according to a segment in the surround circle the attacker is designated to"));
}
