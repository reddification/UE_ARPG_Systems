// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FGameplayTag;

void SendMessageToAIController(AActor* Actor, const FGameplayTag& MessageTag, bool bWasSuccessfull);
