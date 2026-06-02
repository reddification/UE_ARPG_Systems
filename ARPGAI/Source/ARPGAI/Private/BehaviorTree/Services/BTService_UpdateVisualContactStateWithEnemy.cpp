#include "BehaviorTree/Services/BTService_UpdateVisualContactStateWithEnemy.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcSquadMemberComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTService_UpdateVisualContactStateWithEnemy::UBTService_UpdateVisualContactStateWithEnemy()
{
	NodeName = "Update visual contact state with enemy";
	OutNpcSeesEnemyBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutNpcSeesEnemyBBKey));
	OutEnemySeesNpcBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutEnemySeesNpcBBKey));
	
	OutDotProduct_NpcFVToTarget_BBKey.AllowNoneAsValue(true);
	OutDotProduct_NpcFVToTarget_BBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutDotProduct_NpcFVToTarget_BBKey));
	OutDotProduct_TargetFVToNpc_BBKey.AllowNoneAsValue(true);
	OutDotProduct_TargetFVToNpc_BBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutDotProduct_TargetFVToNpc_BBKey));
	OutVisualContactDurationBBKey.AllowNoneAsValue(true);
	OutVisualContactDurationBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutVisualContactDurationBBKey));
	OutLastSeenTargetLocationBBKey.AllowNoneAsValue(true);
	OutLastSeenTargetLocationBBKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutLastSeenTargetLocationBBKey));
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, TargetBBKey), AActor::StaticClass());
	bNotifyCeaseRelevant = true;
	bNotifyBecomeRelevant = true;
}

void UBTService_UpdateVisualContactStateWithEnemy::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                            float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	bool bPreviousNpcSeesEnemy = Blackboard->GetValueAsBool(OutNpcSeesEnemyBBKey.SelectedKeyName);
	auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	auto BTMemory = reinterpret_cast<FBTMemory_UpdateVisualContactState*>(NodeMemory);
	auto NpcPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (Target == nullptr)
	{
		Blackboard->SetValueAsBool(OutNpcSeesEnemyBBKey.SelectedKeyName, false);
		Blackboard->SetValueAsBool(OutEnemySeesNpcBBKey.SelectedKeyName, false);
		
		if (!OutDotProduct_NpcFVToTarget_BBKey.IsNone())
			Blackboard->SetValueAsFloat(OutDotProduct_NpcFVToTarget_BBKey.SelectedKeyName, -FLT_MAX);
		if (!OutDotProduct_TargetFVToNpc_BBKey.IsNone())
			Blackboard->SetValueAsFloat(OutDotProduct_TargetFVToNpc_BBKey.SelectedKeyName, -FLT_MAX);
		if (!OutVisualContactDurationBBKey.IsNone())
			Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, 0.f);
		
		return;
	}

	bool bNpcCanSeeEnemy = false;

	const FVector NpcLocation = NpcPawn->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	const float NpcToEnemyDP = NpcPawn->GetActorForwardVector() | (TargetLocation - NpcLocation).GetSafeNormal();
	if (NpcToEnemyDP >= NpcSeesEnemyDotProductThreshold)
	{
		FActorPerceptionBlueprintInfo TargetPerception;
		bool bHasTargetPerception = OwnerComp.GetAIOwner()->GetAIPerceptionComponent()->GetActorsPerception(Target, TargetPerception);
		if (bHasTargetPerception)
		{
			auto SightSenseId = UAISense::GetSenseID(UAISense_Sight::StaticClass());
			
			for (const auto& PerceptionData : TargetPerception.LastSensedStimuli)
			{
				if (PerceptionData.Type == SightSenseId)
				{
					bNpcCanSeeEnemy = PerceptionData.IsActive() || !PerceptionData.IsExpired() && PerceptionData.GetAge() < SightLostDelay;
					break;
				}
			}
		}
	}

	bool bEnemyCanSeeNpc = false;
	if (bNpcCanSeeEnemy)
	{
		if (!OutDotProduct_NpcFVToTarget_BBKey.IsNone())
			Blackboard->SetValueAsFloat(OutDotProduct_NpcFVToTarget_BBKey.SelectedKeyName, NpcToEnemyDP);
		
		float DotProduct_TargetFVToNpc = Target->GetActorForwardVector() | (NpcLocation - TargetLocation).GetSafeNormal();
		if (!OutDotProduct_TargetFVToNpc_BBKey.IsNone())
			Blackboard->SetValueAsFloat(OutDotProduct_TargetFVToNpc_BBKey.SelectedKeyName, DotProduct_TargetFVToNpc);
		
		BTMemory->LastSeenLocation = TargetLocation;
		if (!OutLastSeenTargetLocationBBKey.IsNone())
			Blackboard->SetValueAsVector(OutLastSeenTargetLocationBBKey.SelectedKeyName, BTMemory->LastSeenLocation);
		
		if (DotProduct_TargetFVToNpc >= EnemyCanSeeNpcDotProductThreshold)
		{
			FVector EnemyEyesLocation = TargetLocation + FVector::UpVector * 75.f;
			FCollisionQueryParams CollisionQueryParams;
			CollisionQueryParams.AddIgnoredActor(NpcPawn);
			CollisionQueryParams.AddIgnoredActor(Target);
			FHitResult Hit;
			bEnemyCanSeeNpc = !Target->GetWorld()->LineTraceSingleByChannel(Hit, EnemyEyesLocation, NpcPawn->GetActorLocation() + FVector::UpVector * 50.f,
				ECC_Visibility, CollisionQueryParams);
		}
	}	

	if (!OutVisualContactDurationBBKey.IsNone())
	{
		float CurrentDirectVisualContactDuration = Blackboard->GetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName);
		if (bNpcCanSeeEnemy && bEnemyCanSeeNpc)
			Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, CurrentDirectVisualContactDuration + DeltaSeconds);
		else if (CurrentDirectVisualContactDuration >= 0.f)
			Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, FMath::Max(0.f, CurrentDirectVisualContactDuration - DeltaSeconds * VisualContactTimerDecayRate));
	}
	
	Blackboard->SetValueAsBool(OutNpcSeesEnemyBBKey.SelectedKeyName, bNpcCanSeeEnemy);
	Blackboard->SetValueAsBool(OutEnemySeesNpcBBKey.SelectedKeyName, bEnemyCanSeeNpc);
}

void UBTService_UpdateVisualContactStateWithEnemy::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto Blackboard = OwnerComp.GetBlackboardComponent();
	
	if (!OutVisualContactDurationBBKey.IsNone())
		Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, 0);
}

void UBTService_UpdateVisualContactStateWithEnemy::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsBool(OutNpcSeesEnemyBBKey.SelectedKeyName, false);
		Blackboard->SetValueAsBool(OutEnemySeesNpcBBKey.SelectedKeyName, false);
		if (!OutDotProduct_TargetFVToNpc_BBKey.IsNone())
			Blackboard->SetValueAsFloat(OutDotProduct_TargetFVToNpc_BBKey.SelectedKeyName, -FLT_MAX);

		if (!OutDotProduct_NpcFVToTarget_BBKey.IsNone())
			Blackboard->SetValueAsFloat(OutDotProduct_NpcFVToTarget_BBKey.SelectedKeyName, -FLT_MAX);
			
		if (!OutVisualContactDurationBBKey.IsNone())
			Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, 0);
		
		if (!OutLastSeenTargetLocationBBKey.IsNone())
			Blackboard->ClearValue(OutLastSeenTargetLocationBBKey.SelectedKeyName);
	}
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

FString UBTService_UpdateVisualContactStateWithEnemy::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("Target BB: %s\n[out]Npc sees enemy BB: %s\n[out]Enemy sees NPC BB: %s\n[out]Npc to target DP BB: %s\n[out]Target to NPC DP BB: %s\n[out]Last seen target location BB: %s\nNpc sight dot product threshold = %.2f"),
		*TargetBBKey.SelectedKeyName.ToString(), *OutNpcSeesEnemyBBKey.SelectedKeyName.ToString(), *OutEnemySeesNpcBBKey.SelectedKeyName.ToString(),
		*OutDotProduct_NpcFVToTarget_BBKey.SelectedKeyName.ToString(), *OutDotProduct_TargetFVToNpc_BBKey.SelectedKeyName.ToString(),
		*OutLastSeenTargetLocationBBKey.SelectedKeyName.ToString(), EnemyCanSeeNpcDotProductThreshold);
	
	return FString::Printf(TEXT("%s\n%s"), *Result, *Super::GetStaticDescription());
}

void UBTService_UpdateVisualContactStateWithEnemy::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	if (auto BB = Asset.GetBlackboardAsset())
	{
		OutDotProduct_TargetFVToNpc_BBKey.ResolveSelectedKey(*BB);
		OutDotProduct_NpcFVToTarget_BBKey.ResolveSelectedKey(*BB);
		OutVisualContactDurationBBKey.ResolveSelectedKey(*BB);
		OutLastSeenTargetLocationBBKey.ResolveSelectedKey(*BB);
	}
}
