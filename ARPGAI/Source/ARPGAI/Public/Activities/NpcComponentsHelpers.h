#pragma once

class AAIController;
class INpcGoalManager;
class UNpcFlowComponent;
class UNpcComponent;
class UNpcActivityInstance;
class UBehaviorTreeComponent;
class UNpcAttitudesComponent;
class UNpcCombatLogicComponent;
class UNpcBehaviorEvaluatorComponent;

ARPGAI_API UNpcComponent* GetNpcComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcComponent* GetNpcComponent(const APawn* Pawn);

ARPGAI_API INpcGoalManager* GetNpcGoalManager(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcFlowComponent* GetNpcFlowComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcBehaviorEvaluatorComponent* GetNpcBehaviorEvaluatorComponent(const UBehaviorTreeComponent& OwnerComp);

ARPGAI_API UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const APawn* Pawn);

ARPGAI_API UNpcAttitudesComponent* GetNpcAttitudesComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcAttitudesComponent* GetNpcAttitudesComponent(const AAIController* AIController);
ARPGAI_API UNpcAttitudesComponent* GetNpcAttitudesComponent(const APawn* Pawn);
