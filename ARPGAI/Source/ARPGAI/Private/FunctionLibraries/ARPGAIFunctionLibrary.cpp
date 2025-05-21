// 


#include "FunctionLibraries/ARPGAIFunctionLibrary.h"

#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"
#include "Interfaces/NpcAliveCreature.h"

bool UARPGAIFunctionLibrary::IsActorEnemyForNpc(AActor* Npc, AActor* TestedActor)
{
	return GetNpcAttitude(Npc, TestedActor).MatchesTag(AIGameplayTags::AI_Attitude_Hostile);
}

FGameplayTag UARPGAIFunctionLibrary::GetNpcAttitude(AActor* Npc, AActor* TestedActor)
{
	auto NpcComponent = Npc->FindComponentByClass<UNpcComponent>();
	if (!NpcComponent)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red,
			FString::Printf(TEXT("UARPGAIFunctionLibrary::IsActorEnemyForNpc: Npc %s doesn't have UNpcComponent"), *Npc->GetName()));
		return FGameplayTag::EmptyTag;
	}

	return NpcComponent->GetAttitude(TestedActor);
}

bool UARPGAIFunctionLibrary::IsActorAlive(AActor* TestActor)
{
	auto NpcAliveCreature = Cast<INpcAliveCreature>(TestActor);
	return NpcAliveCreature ? NpcAliveCreature->IsNpcActorAlive() : false;
}

FGameplayTag UARPGAIFunctionLibrary::RequestGameplayTagFromName(const FName& TagName, bool bErrorOnFail)
{
	return FGameplayTag::RequestGameplayTag(TagName, bErrorOnFail);
}
