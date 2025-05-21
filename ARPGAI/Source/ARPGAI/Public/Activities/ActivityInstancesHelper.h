#pragma once

class UNpcActivityComponent;
class UNpcComponent;
class UNpcActivityInstance;
class UBehaviorTreeComponent;

UNpcComponent* GetNpcComponent(const UBehaviorTreeComponent& OwnerComp);
UNpcActivityComponent* GetNpcActivityComponent(const UBehaviorTreeComponent& OwnerComp);