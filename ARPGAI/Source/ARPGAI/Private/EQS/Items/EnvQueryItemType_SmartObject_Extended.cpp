// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS/Items/EnvQueryItemType_SmartObject_Extended.h"

UEnvQueryItemType_SmartObject_Extended::UEnvQueryItemType_SmartObject_Extended()
{
	ValueSize = sizeof(FSmartObjectSlotEQSItem_Extended);
}

const FSmartObjectSlotEQSItem_Extended& UEnvQueryItemType_SmartObject_Extended::GetValue(const uint8* RawData)
{
	return GetValueFromMemory<FSmartObjectSlotEQSItem_Extended>(RawData);
}

void UEnvQueryItemType_SmartObject_Extended::SetValue(uint8* RawData, const FSmartObjectSlotEQSItem_Extended& Value)
{
	return SetValueInMemory<FSmartObjectSlotEQSItem_Extended>(RawData, Value);
}

FVector UEnvQueryItemType_SmartObject_Extended::GetItemLocation(const uint8* RawData) const
{
	auto SlotData = GetValueFromMemory<FSmartObjectSlotEQSItem_Extended>(RawData);
	return SlotData.Location;
}

FRotator UEnvQueryItemType_SmartObject_Extended::GetItemRotation(const uint8* RawData) const
{
	auto SlotData = GetValueFromMemory<FSmartObjectSlotEQSItem_Extended>(RawData);
	return SlotData.Rotation;
}
