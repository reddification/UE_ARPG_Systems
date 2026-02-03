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

	UFUNCTION(BlueprintCallable)
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
	void SetBaseAttitudes(const FNpcAttitudes& Attitudes);

	bool OnHitReceivedFromActor(const AActor* DamageCauser);
	virtual bool IsHostile(const AActor* Actor);
	virtual bool IsFriendly(const AActor* Actor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	FGameplayTag CurrentAttitudePreset = FGameplayTag::EmptyTag;
	FGameplayTag CurrentTemporaryAttitudePreset = FGameplayTag::EmptyTag;

	TMap<FGameplayTag, float> NpcAttitudesDurationGameTime;
	mutable TMap<TWeakObjectPtr<const AActor>, FTemporaryCharacterAttitudeMemory> TemporaryCharacterAttitudes;

	FNpcAttitudes BaseAttitudes;
	FNpcAttitudes CustomAttitudes;

	FDataTableRowHandle NpcDTRH;
	FGameplayTag NpcId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CleanupRememberedHitsInterval = 2.f;

	const FNpcDTR* GetNpcDTR() const; 
	
private:
	void SetAttitudePresetInternal(const FGameplayTag& InAttitudePreset);
	void CleanRememberedHitsFromCharacters();
	
	struct FReceivedHitsCountMemory
	{
		int Count;
		FDateTime ForgetAtGameTime = 0.f;
	};
	
	TMap<FGameplayTag, int> ForgivableCountOfHitsForAttitude;
	TMap<FGameplayTag, float> RememberHitsFromCharactersDurationsGTH;
	TMap<TWeakObjectPtr<const AActor>, FReceivedHitsCountMemory> ReceivedHitsFromCharacters; 
	FTimerHandle ForgiveAttacksFromNonHostilesTimer;
};
