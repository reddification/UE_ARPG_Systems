

#include "BehaviorTree/Services/BTService_ActivateAbility.h"

#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"

class UMobAbilityPayload;

UBTService_ActivateAbility::UBTService_ActivateAbility()
{
	NodeName = "Activate Ability";
}

void UBTService_ActivateAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponent(OwnerComp);
	if (OwnerAbilitySystemComponent == NULL)
	{
		return;
	}
	
	bool bCanActivateAbility = false;
	AActor* OwnerActor = OwnerComp.GetAIOwner()->GetPawn();
	if (CanActivateAbility(OwnerComp, NodeMemory) == false) return;
	
	if(!TagFilterQuery.IsEmpty())
	{
		if(IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(OwnerActor))
		{
			FGameplayTagContainer OwnerTagContainer;
			TagAssetInterface->GetOwnedGameplayTags(OwnerTagContainer);
			bCanActivateAbility = TagFilterQuery.Matches(OwnerTagContainer);
		}
	}
	else
	{
		bCanActivateAbility = true;
	}
	
	if(bCanActivateAbility)
	{
		FGameplayEventData Data;
		Data.EventTag = AbilityTriggerEventTag;
		Data.Instigator = OwnerActor;

		TrySendAbilityGameplayEvent(OwnerActor, AbilityTriggerEventTag, Data);
	}
}

bool UBTService_ActivateAbility::CanActivateAbility(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return true;
}

bool UBTService_ActivateAbility::TrySendAbilityGameplayEvent(AActor* TargetActor,
                                                             const FGameplayTag& Tag, const FGameplayEventData& Payload) const
{
	if(TargetActor)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, Tag, Payload);
		return true;
	}
	return false;
}

UAbilitySystemComponent* UBTService_ActivateAbility::GetAbilitySystemComponent(const UBehaviorTreeComponent& OwnerComp) const
{
	if(IAbilitySystemInterface* MobCharacter = Cast<IAbilitySystemInterface>(OwnerComp.GetAIOwner()->GetPawn()))
	{
		return MobCharacter->GetAbilitySystemComponent();
	}
	
	return nullptr;
}

FString UBTService_ActivateAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Service triggering event by the tag: %s\nGameplay tag filters: %s\n%s"),
		*AbilityTriggerEventTag.ToString(), *TagFilterQuery.GetDescription(), *Super::GetStaticDescription());
}