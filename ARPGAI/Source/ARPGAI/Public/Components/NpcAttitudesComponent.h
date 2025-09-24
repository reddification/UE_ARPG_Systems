#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Data/AiDataTypes.h"
#include "Data/NpcDTR.h"
#include "NpcAttitudesComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPGAI_API UNpcAttitudesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcAttitudesComponent();

	void InitializeNpcAttitudes(const FGameplayTag& NpcId, const FDataTableRowHandle& NpcDTRH);
	
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetAttitude(const AActor* Actor) const;

	void AddTemporaryCharacterAttitude(const AActor* Character, const FGameplayTag& Attitude, bool bShareableWithAllies);

	void SetAttitudePreset(const FGameplayTag& InAttitudePreset);
	void ClearAttitudePreset();
	void SetTemporaryAttitudePreset(const FGameplayTag& InAttitudePreset);
	void ResetTemporaryAttitudePreset();
	const FGameplayTag& GetCurrentAttitudePreset() const;

	void SetHostile(AActor* ToActor, bool bLethal, bool bShareableWithAllies);
	void ShareAttitudes(UNpcAttitudesComponent* OtherNpcAttitudesComponent) const;

protected:
	virtual void BeginPlay() override;

	FGameplayTag CurrentAttitudePreset = FGameplayTag::EmptyTag;
	FGameplayTag CurrentTemporaryAttitudePreset = FGameplayTag::EmptyTag;

	TMap<FGameplayTag, float> NpcAttitudesDurationGameTime;
	mutable TMap<TWeakObjectPtr<const AActor>, FTemporaryCharacterAttitudeMemory> TemporaryCharacterAttitudes;

	FNpcAttitudes BaseAttitudes;
	FNpcAttitudes CustomAttitudes;

	FDataTableRowHandle NpcDTRH;
	FGameplayTag NpcId;

	const FNpcDTR* GetNpcDTR() const; 
	
private:
	void SetAttitudePresetInternal(const FGameplayTag& InAttitudePreset);
};
