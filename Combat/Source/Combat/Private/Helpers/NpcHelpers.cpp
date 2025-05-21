// Fill out your copyright notice in the Description page of Project Settings.


#include "Helpers/NpcHelpers.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameplayTagContainer.h"

void SendMessageToAIController(AActor* Actor, const FGameplayTag& MessageTag, bool bWasSuccess)
{
	const APawn* AICharacter = Cast<APawn>(Actor);
	if(!AICharacter) return;
		
	const AAIController* AIController = Cast<AAIController>(AICharacter->GetController());
	if(!AIController) return;

	UBrainComponent* Brain = AIController->GetBrainComponent();
	if(!Brain) return;	
		
	FAIMessage Message;
	Message.MessageName = MessageTag.GetTagName();
	Message.Status = bWasSuccess ? FAIMessage::Success : FAIMessage::Failure;
	Brain->HandleMessage(Message);
}
