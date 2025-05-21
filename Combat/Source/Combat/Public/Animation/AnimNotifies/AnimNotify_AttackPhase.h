// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Data/CombatDataTypes.h"
#include "AnimNotify_AttackPhase.generated.h"

UCLASS()
class COMBAT_API UAnimNotify_AttackPhase : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeleeAttackPhase AttackPhase;

	// relevant only for windup. Used to tell NPCs what kind of attack this is
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMeleeAttackType WindupAttackTrajectory;
};
