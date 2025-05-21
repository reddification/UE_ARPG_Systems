// 


#include "BehaviorTree/Services/BTService_CatchUp.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/NpcCombatTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Npc.h"
#include "Interfaces/NpcSystemGameMode.h"
#include "Perception/AISense_Hearing.h"

UBTService_CatchUp::UBTService_CatchUp()
{
	NodeName = "Catch up with target";
	TargetBBKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CatchUp, TargetBBKey), AActor::StaticClass());
	DistanceToTargetBBKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_CatchUp, DistanceToTargetBBKey));
	
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_CatchUp::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	auto BTMemory = reinterpret_cast<FBTMemory_CatchUp*>(NodeMemory);
	float WorldTime = OwnerComp.GetWorld()->GetTimeSeconds();
	BTMemory->NextDrawAttentionAt = WorldTime + FMath::RandRange(DrawAttentionCooldown * 0.75f, DrawAttentionCooldown * 1.25f);
}

void UBTService_CatchUp::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	if (auto Npc = Cast<INpc>(Pawn))
		Npc->ResetForcedMoveSpeed();

	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

void UBTService_CatchUp::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	auto Blackboard = OwnerComp.GetBlackboardComponent();
	auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBBKey.SelectedKeyName));
	if (!ensure(Target))
		return;

	auto Pawn = OwnerComp.GetAIOwner()->GetPawn();
	auto Npc = Cast<INpc>(Pawn);
	if (!ensure(Npc))
		return;
	
	FBTMemory_CatchUp* BTMemory = reinterpret_cast<FBTMemory_CatchUp*>(NodeMemory);
	float DistanceToTarget = Blackboard->GetValueAsFloat(DistanceToTargetBBKey.SelectedKeyName);
	float WorldTime = OwnerComp.GetWorld()->GetTimeSeconds();

	if (WorldTime > BTMemory->NextDrawAttentionAt && DistanceToTarget < MaxRangeToDrawAttention)
	{
		UAISense_Hearing::ReportNoiseEvent(Pawn, Pawn->GetActorLocation(), Loudness, Pawn, PhraseHeardAtRange, DrawAttentionPhraseId.GetTagName());
		auto NpcGameMode = Cast<INpcSystemGameMode>(GetWorld()->GetAuthGameMode());
		if (ensure(NpcGameMode))
			NpcGameMode->ReportNpcSpeak(Pawn, Npc->GetNpcIdTag(), DrawAttentionPhraseId, PhraseHeardAtRange * Loudness);

		if (DrawAttentionGestureId.IsValid())
			Npc->PerformNpcGesture(DrawAttentionGestureId);

		BTMemory->NextDrawAttentionAt = WorldTime + FMath::RandRange(DrawAttentionCooldown * 0.75f, DrawAttentionCooldown * 1.25f);
	}

	if (bUpdateSpeed)
	{
		const float TargetSpeed = Target->GetVelocity().Size();
		Npc->SetForcedMoveSpeed(TargetSpeed * RelativeSpeedScale);
	}
}

FString UBTService_CatchUp::GetStaticDescription() const
{
	FString Result = FString::Printf(TEXT("Catch up with %s\nDistance to target BB: %s"), *TargetBBKey.SelectedKeyName.ToString(),
		*DistanceToTargetBBKey.SelectedKeyName.ToString());

	if (bDrawAttention)
		Result += FString::Printf(TEXT("\nDraw attention at range: %.2f\nPhrase: %s\nGesture: %s"),
			MaxRangeToDrawAttention, *DrawAttentionPhraseId.ToString(), *DrawAttentionGestureId.ToString());

	if (bUpdateSpeed)
		Result += FString::Printf(TEXT("\nUpdate speed relatively to target scale = %.2f)"), RelativeSpeedScale);
	
	Result += "\n";
	Result += Super::GetStaticDescription();
	
	return Result;
}
