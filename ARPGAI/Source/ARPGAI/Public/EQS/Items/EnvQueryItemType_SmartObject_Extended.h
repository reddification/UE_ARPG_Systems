// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvQueryItemType_SmartObject.h"
#include "AITypes.h"
#include "EnvQueryItemType_SmartObject_Extended.generated.h"

struct FSmartObjectSlotEQSItem_Extended : public FSmartObjectSlotEQSItem
{
	FRotator Rotation;
	FORCEINLINE operator FRotator() const { return Rotation; }

	FSmartObjectSlotEQSItem_Extended(const FVector& InLocation, const FSmartObjectHandle& InSmartObjectHandle,
									 const FSmartObjectSlotHandle& InSlotHandle, const FRotator& Rotation)
		: FSmartObjectSlotEQSItem(InLocation, InSmartObjectHandle, InSlotHandle),
		  Rotation(Rotation)
	{
	}

	void Reset()
	{
		Location = FAISystem::InvalidLocation;
		Rotation = FAISystem::InvalidRotation;
		SmartObjectHandle.Invalidate();
		SlotHandle.Invalidate();
	}

	bool IsValid() const { return SmartObjectHandle.IsValid() && SlotHandle.IsValid(); };

};

UCLASS()
class ARPGAI_API UEnvQueryItemType_SmartObject_Extended : public UEnvQueryItemType_VectorBase
{
	GENERATED_BODY()

public:
	typedef FSmartObjectSlotEQSItem_Extended FValueType;

	UEnvQueryItemType_SmartObject_Extended();

	static const FSmartObjectSlotEQSItem_Extended& GetValue(const uint8* RawData);
	static void SetValue(uint8* RawData, const FSmartObjectSlotEQSItem_Extended& Value);
	
	virtual FVector GetItemLocation(const uint8* RawData) const override;
	virtual FRotator GetItemRotation(const uint8* RawData) const override;
};
