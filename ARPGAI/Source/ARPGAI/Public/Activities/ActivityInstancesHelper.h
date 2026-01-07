#pragma once

class AAIController;
class INpcGoalManager;
class UNpcFlowComponent;
class UNpcComponent;
class UNpcActivityInstance;
class UBehaviorTreeComponent;

UNpcComponent* GetNpcComponent(const UBehaviorTreeComponent& OwnerComp);
INpcGoalManager* GetNpcGoalManager(const UBehaviorTreeComponent& OwnerComp);
UNpcFlowComponent* GetNpcFlowComponent(const UBehaviorTreeComponent& OwnerComp);
class UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const UBehaviorTreeComponent& OwnerComp);
class UNpcBehaviorEvaluatorComponent* GetNpcBehaviorEvaluatorComponent(const UBehaviorTreeComponent& OwnerComp);
class UNpcAttitudesComponent* GetNpcAttitudesComponent(const UBehaviorTreeComponent& OwnerComp);
UNpcAttitudesComponent* GetNpcAttitudesComponent(const AAIController* AIController);