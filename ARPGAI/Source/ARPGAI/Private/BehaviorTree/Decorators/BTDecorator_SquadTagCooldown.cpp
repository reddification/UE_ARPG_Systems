// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Decorators/BTDecorator_SquadTagCooldown.h"

#include "AIController.h"
#include "Components/NpcComponent.h"
#include "Subsystems/NpcSquadSubsystem.h"

UBTDecorator_SquadTagCooldown::UBTDecorator_SquadTagCooldown(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Squad Tag Cooldown";
	bNotifyActivation = true;
	bNotifyDeactivation = true;
}

bool UBTDecorator_SquadTagCooldown::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	return OwnerComp.GetTagCooldownEndTime(CooldownTag) <= OwnerComp.GetAIOwner()->GetWorld()->GetTimeSeconds();
}

void UBTDecorator_SquadTagCooldown::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	Super::OnNodeActivation(SearchData);
	if (bOnNodeActivation)
		SetCooldownForSquadMembers(SearchData.OwnerComp);
}

void UBTDecorator_SquadTagCooldown::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData,
                                                       EBTNodeResult::Type NodeResult)
{
	// order matters for these 2 "if"s below. First set cooldown for allies - then to self 
	if (bOnNodeDeactivation)
		SetCooldownForSquadMembers(SearchData.OwnerComp);

	if (bActivateCooldownToSelfOnDeactivation)
		SearchData.OwnerComp.AddCooldownTagDuration(CooldownTag, CooldownDuration.GetValue(SearchData.OwnerComp), false);
	
	Super::OnNodeDeactivation(SearchData, NodeResult);
}

void UBTDecorator_SquadTagCooldown::SetCooldownForSquadMembers(const UBehaviorTreeComponent& BehaviorComp) const
{
	auto World = BehaviorComp.GetWorld();
	if (!IsValid(World))
		return;
	
	if (BehaviorComp.GetTagCooldownEndTime(CooldownTag) > World->GetTimeSeconds())
		return;
	
	auto AIController = BehaviorComp.GetAIOwner();
	if (AIController == nullptr)
		return;
	
	auto Pawn = AIController->GetPawn();
	if (Pawn == nullptr)
		return;
	
	auto NpcComponent = Pawn->FindComponentByClass<UNpcComponent>();
	if (NpcComponent == nullptr)
		return;

	auto NpcSquadSubsystem = UNpcSquadSubsystem::Get(Pawn);
	const FVector PawnLocation = Pawn->GetActorLocation();
	const auto& SquadMembers = NpcSquadSubsystem->GetAllies(Pawn, false, true);
	const float DistanceLimitSq = DistanceToAllyThreshold * DistanceToAllyThreshold;
	if (SquadMembers.Num() > 0)
	{
		for (const auto SquadMember : SquadMembers)
		{
			if (Pawn == SquadMember)
				continue;

			if (bLimitByRange)
			{
				const float RangeSq = (SquadMember->GetActorLocation() - PawnLocation).SizeSquared();
				if (RangeSq > DistanceLimitSq)
					continue;
			}
			
			auto AllyBrainComponent = SquadMember->GetController()->FindComponentByClass<UBehaviorTreeComponent>();
			if (AllyBrainComponent)
				AllyBrainComponent->AddCooldownTagDuration(CooldownTag, CooldownDuration.GetValue(BehaviorComp), false);
		}
	}
}

FString UBTDecorator_SquadTagCooldown::GetStaticDescription() const
{
	FString Base = FString::Printf(TEXT("%s: lock for %s\n%s"),
		*CooldownTag.ToString(), *CooldownDuration.ToString(),
		bActivateCooldownToSelfOnDeactivation ? TEXT("Add cooldown to self on deactivation") : TEXT("Doesn't add cooldown to self"));

	if (bOnNodeActivation)
		Base = Base.Append(TEXT("\nOn activation"));
	
	if (bOnNodeDeactivation)
		Base = Base.Append(TEXT("\nOn deactivation"));
	
	return bLimitByRange
		? FString::Printf(TEXT("Only for allies in %.2f range\n%s"), DistanceToAllyThreshold, *Base)
		: Base;
}
