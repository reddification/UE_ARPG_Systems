// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_AttackPhase.h"
#include "Components/MeleeCombatComponent.h"
#include "Data/CombatDataTypes.h"

void UAnimNotify_AttackPhase::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	auto MeshOwner = MeshComp->GetOwner();
	if (!MeshOwner)
		return;
	
	auto MeleeCombatComponent = MeshOwner->FindComponentByClass<UMeleeCombatComponent>();
	if (!MeleeCombatComponent)
		return;
	
	switch (AttackPhase)
	{
		case EMeleeAttackPhase::None: ensure(false); break;
		case EMeleeAttackPhase::WindUp:
			MeleeCombatComponent->BeginWindUp(TotalDuration, Animation->GetUniqueID(), WindupAttackTrajectory);
			break;
		case EMeleeAttackPhase::Release:
			MeleeCombatComponent->BeginRelease(TotalDuration, Animation->GetUniqueID());
			break;
		case EMeleeAttackPhase::Recover:
			MeleeCombatComponent->BeginRecover(TotalDuration, Animation->GetUniqueID());
			break;
		default: ensure(false); break;
	}
}

void UAnimNotify_AttackPhase::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	auto MeshOwner = MeshComp->GetOwner();
	if (!MeshOwner)
		return;
	
	auto MeleeCombatComponent = MeshOwner->FindComponentByClass<UMeleeCombatComponent>();
	if (!MeleeCombatComponent)
		return;
	
	switch (AttackPhase)
	{
		case EMeleeAttackPhase::None: ensure(false); break;
		case EMeleeAttackPhase::WindUp:
			MeleeCombatComponent->EndWindUp(Animation->GetUniqueID());
			break;
		case EMeleeAttackPhase::Release:
			MeleeCombatComponent->EndRelease(Animation->GetUniqueID());
			break;
		case EMeleeAttackPhase::Recover:
			MeleeCombatComponent->EndRecover(Animation->GetUniqueID());
			break;
		default: ensure(false); break;
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}