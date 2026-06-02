#pragma once

#include "Engine/DataTable.h"
#include "QuestProgress.generated.h"

USTRUCT(BlueprintType)
struct FQuestEventData
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    int TODO_FlowNodesSaveState;
};

USTRUCT(BlueprintType)
struct FQuestProgress
{
    GENERATED_BODY()

    FQuestProgress() = default;
    
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
    
    TWeakObjectPtr<UFlowAsset> FlowInstance;
    
    FQuestProgress(const FQuestProgress& Other)
        : QuestDTRH(Other.QuestDTRH),
          JournalLogs(Other.JournalLogs),
          QuestState(Other.QuestState),
          PendingQuestEvents(Other.PendingQuestEvents),
          CoveredQuestEvents(Other.CoveredQuestEvents),
          CompletedQuestEvents(Other.CompletedQuestEvents),
          FlowInstance(Other.FlowInstance)
    {
    }

    FQuestProgress(FQuestProgress&& Other) noexcept
        : QuestDTRH(std::move(Other.QuestDTRH)),
          JournalLogs(std::move(Other.JournalLogs)),
          QuestState(Other.QuestState),
          PendingQuestEvents(std::move(Other.PendingQuestEvents)),
          CoveredQuestEvents(std::move(Other.CoveredQuestEvents)),
          CompletedQuestEvents(std::move(Other.CompletedQuestEvents)),
          FlowInstance(std::move(Other.FlowInstance))
    {
    }

    FQuestProgress& operator=(const FQuestProgress& Other)
    {
        if (this == &Other)
            return *this;
        QuestDTRH = Other.QuestDTRH;
        JournalLogs = Other.JournalLogs;
        QuestState = Other.QuestState;
        PendingQuestEvents = Other.PendingQuestEvents;
        CoveredQuestEvents = Other.CoveredQuestEvents;
        CompletedQuestEvents = Other.CompletedQuestEvents;
        FlowInstance = Other.FlowInstance;
        return *this;
    }

    FQuestProgress& operator=(FQuestProgress&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        QuestDTRH = std::move(Other.QuestDTRH);
        JournalLogs = std::move(Other.JournalLogs);
        QuestState = Other.QuestState;
        PendingQuestEvents = std::move(Other.PendingQuestEvents);
        CoveredQuestEvents = std::move(Other.CoveredQuestEvents);
        CompletedQuestEvents = std::move(Other.CompletedQuestEvents);
        FlowInstance = std::move(Other.FlowInstance);
        return *this;
    }
};

inline bool operator == (const FQuestProgress& First, const FQuestProgress& Other)
{
    return First.QuestDTRH == Other.QuestDTRH;
}