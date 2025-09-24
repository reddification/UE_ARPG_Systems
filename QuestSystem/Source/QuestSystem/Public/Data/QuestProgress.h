#pragma once

#include "QuestEventDTR.h"
#include "QuestEventTriggerProxy.h"
#include "Engine/DataTable.h"
#include "QuestProgress.generated.h"

USTRUCT(BlueprintType)
struct FQuestEventData
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FDataTableRowHandle QuestTaskDTRH;
    
    UPROPERTY()
    UQuestEventTriggerProxy* OccuredTrigger = nullptr;

    UPROPERTY()
    UQuestEventTriggerProxy* CoveredTrigger = nullptr;
    
    FQuestEventDTR* GetQuestEventDTR() const { return !QuestTaskDTRH.IsNull() ? QuestTaskDTRH.GetRow<FQuestEventDTR>("") : nullptr; }
    void Finalize()
    {
        OccuredTrigger = nullptr;
        CoveredTrigger = nullptr;
    };
};

USTRUCT(BlueprintType)
struct FQuestProgress
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FDataTableRowHandle QuestDTRH;

    UPROPERTY(SaveGame)
    TArray<FText> JournalLogs;
    
    UPROPERTY(SaveGame)
    EQuestState QuestState;
    
    // 06.08.2025 @AK: obsolete properties, TODO remove when FlowGraph is working
    
    UPROPERTY(SaveGame)
    TMap<FName, FQuestEventData> PendingQuestEvents;

    UPROPERTY(SaveGame)
    TMap<FName, FQuestEventData> CoveredQuestEvents;
    
    UPROPERTY(SaveGame)
    TMap<FName, FQuestEventData> CompletedQuestEvents;
};

inline bool operator == (const FQuestProgress& First, const FQuestProgress& Other)
{
    return First.QuestDTRH == Other.QuestDTRH;
}

inline bool operator == (const FQuestEventData& First, const FQuestEventData& Second)
{
    return First.QuestTaskDTRH == Second.QuestTaskDTRH;
}
