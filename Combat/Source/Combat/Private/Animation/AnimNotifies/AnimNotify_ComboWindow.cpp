// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifies/AnimNotify_ComboWindow.h"

#include "Components/MeleeCombatComponent.h"

void UAnimNotify_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (auto Owner = MeshComp->GetOwner())
	{
		if (auto MeleeCombatComponent = Owner->FindComponentByClass<UMeleeCombatComponent>())
		{
			MeleeCombatComponent->StartComboWindow(Animation->GetUniqueID());
		}
	}
}

void UAnimNotify_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (auto Owner = MeshComp->GetOwner())
	{
		if (auto MeleeCombatComponent = Owner->FindComponentByClass<UMeleeCombatComponent>())
		{
			MeleeCombatComponent->EndComboWindow(Animation->GetUniqueID());
		}
	}
}
