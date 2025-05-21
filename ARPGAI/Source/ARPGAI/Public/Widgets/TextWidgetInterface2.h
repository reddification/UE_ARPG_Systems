

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TextWidgetInterface2.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UTextWidgetInterface2 : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPGAI_API ITextWidgetInterface2
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void SetText(const FText& Text);
	virtual void SetText_Implementation(const FText& Text) {}
	
	UFUNCTION(BlueprintNativeEvent)
	void SetTextColor(const FLinearColor& TextColor);
	virtual void SetTextColor_Implementation(const FLinearColor& TextColor) {}
};
