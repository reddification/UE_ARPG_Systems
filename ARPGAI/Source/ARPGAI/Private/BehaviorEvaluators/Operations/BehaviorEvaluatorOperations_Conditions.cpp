#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_Conditions.h"

#include "Activities/NpcComponentsHelpers.h"
#include "BehaviorEvaluators/Operations/BehaviorEvaluatorOperations_DataTypes.h"
#include "Components/Controller/NpcPerceptionComponent.h"
#include "Interfaces/NpcThreat.h"
#include "Subsystems/NpcSquadSubsystem.h"

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	auto Base = Super::Evaluate(Context, Target, CharacterSTM);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid() || !Statement2.IsValid())
		return false;
	
	const bool b1 = Statement1.Get().Evaluate(Context, Target, CharacterSTM);
	const bool b2 = Statement2.Get().Evaluate(Context, Target, CharacterSTM);
	return EvaluateInternal(b1, b2);
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	auto Base = Super::Evaluate(Context, NpcPerceptionComponent);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid() || !Statement2.IsValid())
		return false;
	
	const bool b1 = Statement1.Get().Evaluate(Context, NpcPerceptionComponent);
	const bool b2 = Statement2.Get().Evaluate(Context, NpcPerceptionComponent);
	return EvaluateInternal(b1, b2);
}

FString FBehaviorEvaluatorOperationCondition_LogicalOperation_Binary::ToString(int Indentation) const
{
	auto Base = Super::ToString(Indentation);
	if (!Statement1.IsValid())
		return Base + TEXT("Error! Statement 1 is not set");
	
	if (!Statement2.IsValid())
		return Base + TEXT("Error! Statement 2 is not set");
	
	FString Indentation1Str = FString::ChrN(Indentation + 1, '\t');
	return FString::Printf(TEXT("%s\n%s%s\n%s"), 
		*Statement1.Get().ToString(Indentation), *Indentation1Str, *BinaryOpInfo(), *Statement2.Get().ToString(Indentation));
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Compound::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	if (Statements.IsEmpty())
		return false;
	
	switch (Mode) 
	{
		case EBehaviorEvaluatorConditionCompoundOperation::AND:
			for (const auto& StatementIS : Statements)
				if (!StatementIS.IsValid() || !StatementIS.Get().Evaluate(Context, Target, CharacterSTM))
					return false;

			return true;
		case EBehaviorEvaluatorConditionCompoundOperation::OR:
			for (const auto& StatementIS : Statements)
				if (StatementIS.IsValid() && StatementIS.Get().Evaluate(Context, Target, CharacterSTM))
					return true;

			return false;
		default:
			ensure(false);
			break;
	}
	
	return false;
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Compound::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	if (Statements.IsEmpty())
		return false;
	
	switch (Mode) 
	{
	case EBehaviorEvaluatorConditionCompoundOperation::AND:
		for (const auto& StatementIS : Statements)
			if (!StatementIS.IsValid() || !StatementIS.Get().Evaluate(Context, NpcPerceptionComponent))
				return false;

		return true;
	case EBehaviorEvaluatorConditionCompoundOperation::OR:
		for (const auto& StatementIS : Statements)
			if (StatementIS.IsValid() && StatementIS.Get().Evaluate(Context, NpcPerceptionComponent))
				return true;

		return false;
	default:
		ensure(false);
		break;
	}
	
	return false;
}

FString FBehaviorEvaluatorOperationCondition_LogicalOperation_Compound::ToString(int Indentation) const
{
	FString Description = FString::ChrN(Indentation, '\t') + StaticEnum<EBehaviorEvaluatorConditionCompoundOperation>()->GetDisplayValueAsText(Mode).ToString();
	
	for (const auto& StatementIS : Statements)
	{
		if (StatementIS.IsValid())
			Description += TEXT("\n") + StatementIS.Get().ToString(Indentation + 1);
		else
			Description += FString::Printf(TEXT("\n%sInvalid statement"), *FString::ChrN(Indentation + 1, '\t'));
	}

	return Description;
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Conjunction::EvaluateInternal(bool bStatement1, bool bStatement2) const
{
	return bStatement1 && bStatement2;
}

bool FBehaviorEvaluatorOperationCondition_LogicalOperation_Disjunction::EvaluateInternal(bool bStatement1,
	bool bStatement2) const
{
	return bStatement1 || bStatement2;
}

bool FBehaviorEvaluatorOperationCondition_Unary_Not::Evaluate(const FRelativeOperationContext& Context,
                                                              const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const
{
	auto Base = Super::Evaluate(Context, Target, CharacterSTM);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid())
		return false;
	
	return !Statement1.Get().Evaluate(Context, Target, CharacterSTM);
}

bool FBehaviorEvaluatorOperationCondition_Unary_Not::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	auto Base = Super::Evaluate(Context, NpcPerceptionComponent);
	if (!Base)
		return false;
	
	if (!Statement1.IsValid())
		return false;
	
	return !Statement1.Get().Evaluate(Context, NpcPerceptionComponent);
}

FString FBehaviorEvaluatorOperationCondition_Unary_Not::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("NOT %s"), *Statement1.Get().ToString(Indentation + 1));
}

bool FBehaviorEvaluatorOperationCondition_EvaluatorState::Evaluate(const FRelativeOperationContext& Context,
                                                                   const AActor* Target, const FCharacterShortTermMemory& CharacterSTM) const
{
	return Super::Evaluate(Context, Target, CharacterSTM) && Context.EvaluatorState == RequiredState;
}

bool FBehaviorEvaluatorOperationCondition_EvaluatorState::Evaluate(const FAggregationOperationContext& Context,
	const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return Super::Evaluate(Context, NpcPerceptionComponent) && Context.EvaluatorState == RequiredState;
}

FString FBehaviorEvaluatorOperationCondition_EvaluatorState::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("Evaluator state == %s"), 
		*StaticEnum<EBehaviorEvaluatorState>()->GetDisplayValueAsText(RequiredState).ToString());
}

bool FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	return Super::Evaluate(Context, Target, CharacterSTM)
		&& CharacterSTM.HasVisualDetection() && CharacterSTM.TimeSeen >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration::Evaluate(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	unimplemented();
	return Super::Evaluate(Context, NpcPerceptionComponent);
}

FString FBehaviorEvaluatorOperationCondition_Activation_VisualContactDuration::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("See target longer than %.2fs"), ActivationThreshold);
}

bool FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	return Context.AccumulatedScore >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore::Evaluate(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return Context.AccumulatedScore >= ActivationThreshold;
}

FString FBehaviorEvaluatorOperationCondition_Activation_AccumulatedScore::ToString(int Indentation) const
{
	return Super::ToString(Indentation) + FString::Printf(TEXT("Accumulated score >= %.2fs"), ActivationThreshold);
}

bool FBehaviorEvaluatorOperationCondition_Activation_BehaviorDuration::Evaluate(
	const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	return Context.ActiveBehaviorDuration >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_BehaviorDuration::Evaluate(
	const FAggregationOperationContext& Context, const UNpcPerceptionComponent* NpcPerceptionComponent) const
{
	return Context.ActiveBehaviorDuration >= ActivationThreshold;
}

bool FBehaviorEvaluatorOperationCondition_Activation_Distance::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
                                                                        const FCharacterShortTermMemory& CharacterSTM) const
{
	return CharacterSTM.Distance <= DistanceThreshold;
}

bool FBehaviorEvaluatorOperationCondition_IsAlive::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
                                                            const FCharacterShortTermMemory& CharacterSTM) const
{
	return CharacterSTM.bAlive == bDesiredState;
}

bool FBehaviorEvaluatorOperationCondition_Activation_IsHostile::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
                                                                         const FCharacterShortTermMemory& CharacterSTM) const
{
	return CharacterSTM.bHostile == bDesiredState;
}

bool FBehaviorEvaluatorOperationCondition_InCombatWithAllies::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	auto SquadSubsystem = UNpcSquadSubsystem::Get(Context.Pawn.Get());
	auto MyAllies = SquadSubsystem->GetAllies(Context.Pawn.Get(), true);
	bool bInCombatWithAllies = false;
	for (const auto* Ally: MyAllies)
	{
		auto AllyCombatLogicComponent = GetNpcCombatLogicComponent(Ally);
		if (AllyCombatLogicComponent->HasTarget(Target))
		{
			bInCombatWithAllies = true;
			break;
		}
	}
	
	return bInCombatWithAllies == bDesiredState;
}

bool FBehaviorEvaluatorOperationCondition_LongTermAccumulatedDamage::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	const float RelativeDamage = CharacterSTM.LongTermAccumulatedReceivedDamage / Context.MaxHealth;
	return RelativeDamage >= NormalizedDamageThreshold;
}

bool FBehaviorEvaluatorOperationCondition_HasTags::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	return ActorFilter.Matches(CharacterSTM.CharacterTags);
}

bool FBehaviorEvaluatorOperationCondition_IsPrimaryTarget::Evaluate(const FRelativeOperationContext& Context, const AActor* Target,
	const FCharacterShortTermMemory& CharacterSTM) const
{
	if (bTargetForOwner)
	{
		auto OwnerThreat = Cast<INpcThreat>(Context.Pawn.Get());
		return OwnerThreat->IsPrimaryTarget_NpcThreat(Target, ForBehavior);
	}
	else
	{
		auto TargetThreat = Cast<INpcThreat>(Target);
		return TargetThreat != nullptr ? TargetThreat->IsPrimaryTarget_NpcThreat(Context.Pawn.Get(), ForBehavior) : false;
	}
}

FString FBehaviorEvaluatorOperationCondition_IsPrimaryTarget::ToString(int Indentation) const
{
	FString ConditionDescription = bTargetForOwner ?
		FString::Printf(TEXT("Tested actor is primary target for owner? [%s]"), *ForBehavior.ToString()):
		FString::Printf(TEXT("Owner is primary target for tested actor? [%s]"), *ForBehavior.ToString());
	
	return Super::ToString(Indentation) + ConditionDescription;
}
