// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GameplayCueNotifies/GCN_MobSpawn.h"

#include "Components/NpcComponent.h"
#include "Data/AIGameplayTags.h"

void UGCN_MobSpawn::PostInitProperties()
{
	Super::PostInitProperties();
	// GameplayCueTag = LyraGameplayTags::GameplayCue_AI_Spawn;
}

bool UGCN_MobSpawn::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	return true;
	// UNpcComponent* MobComponent = MyTarget->FindComponentByClass<UNpcComponent>();
	// return MobComponent ? MobComponent->PlaySpawnVFX() : false;
}

bool UGCN_MobSpawn::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	return true;
	// UNpcComponent* MobComponent = MyTarget->FindComponentByClass<UNpcComponent>();
	// return MobComponent ? MobComponent->PlaySpawnVFX() : false;
	// return Super::OnActive_Implementation(MyTarget, Parameters);
}
