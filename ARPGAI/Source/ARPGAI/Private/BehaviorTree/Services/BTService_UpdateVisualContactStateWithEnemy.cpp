// Fill out your copyright notice in the Description page of Project Settings.


#include "BehaviorTree/Services/BTService_UpdateVisualContactStateWithEnemy.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Controller/NpcSquadMemberComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/Npc.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

UBTService_UpdateVisualContactStateWithEnemy::UBTService_UpdateVisualContactStateWithEnemy()
{
	NodeName = "Update visual contact state with enemy";
	OutNpcSeesEnemyBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutNpcSeesEnemyBBKey));
	OutEnemySeesNpcBBKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutEnemySeesNpcBBKey));
	OutDotProduct_NpcFV_EnemyFV_BBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutDotProduct_NpcFV_EnemyFV_BBKey));
	OutVisualContactDurationBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, OutVisualContactDurationBBKey));
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateVisualContactStateWithEnemy, TargetBBKey), AActor::StaticClass());
	bNotifyCeaseRelevant = true;
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
		Blackboard->SetValueAsFloat(OutDotProduct_NpcFV_EnemyFV_BBKey.SelectedKeyName, FLT_MAX);
		Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, 0.f);
		if (bPreviousNpcSeesEnemy)
		{
			auto Npc = Cast<INpc>(NpcPawn);
			Npc->RemoveNpcTags(AIGameplayTags::AI_State_DirectVisualContact.GetTag().GetSingleTagContainer());
			if (BTMemory->LastSeenLocation != FAISystem::InvalidLocation)
			{
				const float DotProductNpcFVToNpcToEnemy = NpcPawn->GetActorForwardVector() | (BTMemory->LastSeenLocation - NpcPawn->GetActorLocation()).GetSafeNormal();
				if (DotProductNpcFVToNpcToEnemy > 0.2f) // if NPC didn't turn back himself
					ReportVisualContactStateChanged(OwnerComp, NpcPawn, ChanceToReportVisualContactLost, AIGameplayTags::AI_Behavior_Combat_Event_LostContact);
			}
		}
		
		return;
	}

	bool bNpcCanSeeEnemy = false;

	const FVector NpcLocation = NpcPawn->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	if ((NpcPawn->GetActorForwardVector() | (TargetLocation - NpcLocation).GetSafeNormal()) >= NpcSeesEnemyDotProductThreshold)
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
					if (PerceptionData.IsActive())
						bNpcCanSeeEnemy = true;
					else if (!PerceptionData.IsExpired() && PerceptionData.GetAge() < SightLostDelay)
						bNpcCanSeeEnemy = true;
					
					break;
				}
			}
		}
	}

	bool bEnemyCanSeeNpc = false;
	if (bNpcCanSeeEnemy)
	{
		const float DotProduct = Target->GetActorForwardVector() | NpcPawn->GetActorForwardVector();
		Blackboard->SetValueAsFloat(OutDotProduct_NpcFV_EnemyFV_BBKey.SelectedKeyName, DotProduct);
		BTMemory->LastSeenLocation = Target->GetActorLocation();
		if (DotProduct < NpcFVToEnemyFVDotProductThreshold)
		{
			FVector EnemyEyesLocation = Target->GetActorLocation() + FVector::UpVector * 75.f;
			FCollisionQueryParams CollisionQueryParams;
			CollisionQueryParams.AddIgnoredActor(NpcPawn);
			CollisionQueryParams.AddIgnoredActor(Target);
			FHitResult Hit;
			bEnemyCanSeeNpc = !Target->GetWorld()->LineTraceSingleByChannel(Hit, EnemyEyesLocation, NpcPawn->GetActorLocation() + FVector::UpVector * 50.f,
				ECC_Visibility, CollisionQueryParams);
		}
	}	

	float CurrentDirectVisualContactDuration = Blackboard->GetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName);
	if (bNpcCanSeeEnemy && bEnemyCanSeeNpc)
		Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, CurrentDirectVisualContactDuration + DeltaSeconds);
	else if (CurrentDirectVisualContactDuration >= 0.f)
		Blackboard->SetValueAsFloat(OutVisualContactDurationBBKey.SelectedKeyName, FMath::Max(0.f, CurrentDirectVisualContactDuration - DeltaSeconds * VisualContactTimerDecayRate));
	
	Blackboard->SetValueAsBool(OutNpcSeesEnemyBBKey.SelectedKeyName, bNpcCanSeeEnemy);
	Blackboard->SetValueAsBool(OutEnemySeesNpcBBKey.SelectedKeyName, bEnemyCanSeeNpc);

	if (bPreviousNpcSeesEnemy != bNpcCanSeeEnemy)
	{
		if (auto AICharacter = Cast<INpc>(NpcPawn))
		{
			if (bNpcCanSeeEnemy)
			{
				AICharacter->GiveNpcTags(AIGameplayTags::AI_State_DirectVisualContact.GetTag().GetSingleTagContainer());
				ReportVisualContactStateChanged(OwnerComp, NpcPawn, ChanceToReportVisualContactAcquired, AIGameplayTags::AI_Noise_Report_VisualContact_Acquired);
			}
			else
			{
				AICharacter->RemoveNpcTags(AIGameplayTags::AI_State_DirectVisualContact.GetTag().GetSingleTagContainer());
			}
		}
	}
}

void UBTService_UpdateVisualContactStateWithEnemy::ReportVisualContactStateChanged(UBehaviorTreeComponent& OwnerComp, APawn* NpcPawn,
	float ChanceToReport, const FGameplayTag& ReportTag)
{
	if (FMath::RandRange(0.f, 1.f) < ChanceToReport)
	{
		const auto WorldTime = NpcPawn->GetWorld()->GetTimeSeconds();
		if (OwnerComp.GetTagCooldownEndTime(ReportTag) < WorldTime)
		{
			// NpcPawn->Multicast_SayPhrase(ReportTag);
			UAISense_Hearing::ReportNoiseEvent(NpcPawn, NpcPawn->GetActorLocation(), 1.f, NpcPawn, 1000.f, ReportTag.GetTagName());
			OwnerComp.AddCooldownTagDuration(ReportTag, 10.f, false);
			auto NpcSquadComponent = NpcPawn->GetController()->FindComponentByClass<UNpcSquadMemberComponent>();
			NpcSquadComponent->AddBehaviorCooldownToAllies(ReportTag, 15.f);
		}
	}
}

void UBTService_UpdateVisualContactStateWithEnemy::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsBool(OutNpcSeesEnemyBBKey.SelectedKeyName, false);
		Blackboard->SetValueAsBool(OutEnemySeesNpcBBKey.SelectedKeyName, false);
	}

	if (auto AIController = OwnerComp.GetAIOwner())
		if (auto AICharacter = Cast<INpc>(AIController->GetPawn()))
			AICharacter->RemoveNpcTags(AIGameplayTags::AI_State_DirectVisualContact.GetTag().GetSingleTagContainer());
	
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

FString UBTService_UpdateVisualContactStateWithEnemy::GetStaticDescription() const
{
	const FString VisualContactTagString = AIGameplayTags::AI_State_DirectVisualContact.GetTag().GetTagName().ToString();
	return FString::Printf(TEXT("Target BB: %s\n[out]Npc sees enemy BB: %s\n[out]Enemy sees NPC BB: %s\n[out]Dot product between NPC FV and enemy FV BB: %s\nNpc sight dot product threshold = %.2f\nGrants tag %s when NPC sees enemy\n%s"),
		*TargetBBKey.SelectedKeyName.ToString(), *OutNpcSeesEnemyBBKey.SelectedKeyName.ToString(), *OutEnemySeesNpcBBKey.SelectedKeyName.ToString(),
		*OutDotProduct_NpcFV_EnemyFV_BBKey.SelectedKeyName.ToString(), NpcFVToEnemyFVDotProductThreshold, *VisualContactTagString, *Super::GetStaticDescription());
}
