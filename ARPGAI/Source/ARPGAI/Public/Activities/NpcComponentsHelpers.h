#pragma once

class UNpcConversationComponent;
class UNpcMemoryComponent;
class UNpcPerceptionComponent;
class UNpcHealingComponent;
class UNpcAreasComponent;
class UNpcBehaviorEvaluatorComponent2;
class URoleplayComponent;
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
ARPGAI_API UNpcFlowComponent* GetNpcFlowComponent(const APawn* Pawn);

ARPGAI_API UNpcBehaviorEvaluatorComponent* GetNpcBehaviorEvaluatorComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcBehaviorEvaluatorComponent2* GetNpcBehaviorEvaluatorComponent_v2(const UBehaviorTreeComponent& OwnerComp);

ARPGAI_API UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcCombatLogicComponent* GetNpcCombatLogicComponent(const APawn* Pawn);

ARPGAI_API UNpcAttitudesComponent* GetNpcAttitudesComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcAttitudesComponent* GetNpcAttitudesComponent(const AAIController* AIController);
ARPGAI_API UNpcAttitudesComponent* GetNpcAttitudesComponent(const APawn* Pawn);

ARPGAI_API URoleplayComponent* GetRoleplayComponent(const AAIController* AIController);

ARPGAI_API UNpcAreasComponent* GetNpcAreasComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcAreasComponent* GetNpcAreasComponent(APawn* Pawn);

ARPGAI_API UNpcHealingComponent* GetNpcHealComponent(const UBehaviorTreeComponent& OwnerComp);

ARPGAI_API UNpcPerceptionComponent* GetNpcShortTermMemoryComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcPerceptionComponent* GetNpcShortTermMemoryComponent(const APawn* OwnerPawn);

ARPGAI_API UNpcMemoryComponent* GetNpcLongTermMemoryComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcMemoryComponent* GetNpcLongTermMemoryComponent(const APawn* OwnerPawn);

ARPGAI_API UNpcConversationComponent* GetNpcConversationComponent(const UBehaviorTreeComponent& OwnerComp);
ARPGAI_API UNpcConversationComponent* GetNpcConversationComponent(const APawn* Pawn);
