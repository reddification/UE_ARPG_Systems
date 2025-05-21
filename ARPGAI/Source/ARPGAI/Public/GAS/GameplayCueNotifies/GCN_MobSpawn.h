// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "GameplayCueNotify_Static.h"
#include "UObject/Object.h"
#include "GCN_MobSpawn.generated.h"

/**
 * 
 */
UCLASS()
class ARPGAI_API UGCN_MobSpawn : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
