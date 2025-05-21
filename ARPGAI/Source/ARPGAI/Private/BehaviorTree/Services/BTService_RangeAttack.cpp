

#include "BehaviorTree/Services/BTService_RangeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

bool UBTService_RangeAttack::FAttackNodeMemory::IsValid() const
{
	return true;
}

UBTService_RangeAttack::UBTService_RangeAttack()
{
	NodeName = "RangedAttack";
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RangeAttack, TargetBBKey), AActor::StaticClass());
	IsAttackOnCooldownBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_RangeAttack, IsAttackOnCooldownBBKey));
	bNotifyBecomeRelevant = 1;
}

void UBTService_RangeAttack::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	FAttackNodeMemory* AttackNodeMemory = reinterpret_cast<FAttackNodeMemory*>(NodeMemory);
	if (AttackNodeMemory->IsValid() == false)
	{
		return;
	}
	if(AttackNodeMemory->CurrentAttackCooldown <= 0.f)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(IsAttackOnCooldownBBKey.SelectedKeyName, false);
	}
	if (AttackNodeMemory->AttackTimeLeft <= 0.f && AttackNodeMemory->CurrentAttackCooldown <= 0.f)
	{
		const AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetBBKey.SelectedKeyName));
		if(TargetActor == nullptr)
		{
			return;
		}
		// Distance to the edge of target's collision
		const float CollisionRadius = TargetActor->GetSimpleCollisionRadius();
		const float Distance = FVector::Distance(TargetActor->GetActorLocation(), OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation()) - CollisionRadius;
		if (Distance <= AttackNodeMemory->AttackRange)
		{
			
			if(AttackNodeMemory->IsAiming == false)
			{
				AttackNodeMemory->IsAiming = true;
			}
			else
			{
				AttackNodeMemory->TakeAimTimeLeft -= DeltaSeconds;
			}
			
			if(AttackNodeMemory->TakeAimTimeLeft <= 0.f)
			{
				const float CooldownTime = FMath::RandRange(AttackNodeMemory->AttackCooldown * (1.f - AttackNodeMemory->AttackCooldownDeviationRatio),
			AttackNodeMemory->AttackCooldown * (1.f + AttackNodeMemory->AttackCooldownDeviationRatio));
				AttackNodeMemory->CurrentAttackCooldown = CooldownTime;
				
				if(APawn* Owner = OwnerComp.GetAIOwner()->GetPawn())
				{
					FGameplayEventData Payload;
					Payload.Instigator = Owner;
					Payload.Target = TargetActor;
					//TODO: Decide what to do with effect magnitude value(nothing?)
					//Payload.EventMagnitude = 1;
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, AttackTag, Payload);
					//TODO: This was calculated with gesture system, attack returned how long the animation of attack will play. Now it's just a stub. FIX!
					AttackNodeMemory->AttackTimeLeft = 2.0f;
				}
				AttackNodeMemory->IsAiming = false;
			}
		}
		
	}
	else if (AttackNodeMemory->AttackTimeLeft >= 0.f)
	{
		AttackNodeMemory->AttackTimeLeft -= DeltaSeconds;
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(IsAttackOnCooldownBBKey.SelectedKeyName, true);
		AttackNodeMemory->CurrentAttackCooldown -= DeltaSeconds;
		AttackNodeMemory->AttackTimeLeft -= DeltaSeconds;
	}
}

FString UBTService_RangeAttack::GetStaticDescription() const
{
	return 	FString::Printf(TEXT("%s\nAttack tag: %s\nTake aim tag: %s"),
		*Super::GetStaticDescription(), *AttackTag.ToString(), *AimTag.ToString());
}

uint16 UBTService_RangeAttack::GetInstanceMemorySize() const
{
	return sizeof(FAttackNodeMemory);
}

void UBTService_RangeAttack::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	const AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (IsValid(TargetActor) == false)
	{
		return;
	}
	
	//TODO Gesture and Combat components need to be implemented as a GameplayAbilities
	FAttackNodeMemory* AttackMemory = (FAttackNodeMemory*)NodeMemory;
	AttackMemory->CurrentAttackCooldown = 0.f;
	AttackMemory->AttackRange = 1500.0f;
	AttackMemory->AttackCooldown = 3.0f;
	AttackMemory->AttackCooldownDeviationRatio = 0.5f;
	// AttackMemory->MobGestureComponent = GestureComponent;
	// AttackMemory->MobCombatComponent = CombatComponent;
}
