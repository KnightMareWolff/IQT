// IQT/Source/IQT/Private/Internal/IQT_PriorityQueueInternal.h
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "IQT_DynAINode.h" 
#include "HAL/CriticalSection.h" 
#include "IQT_DataTypes.h"       

/**
 * UIQT_PriorityQueueInternal: Implementa uma fila de prioridade usando uma lista duplamente encadeada.
 * Esta fila é thread-safe e gerencia seus próprios nós internos.
 */
class UIQT_PriorityQueueInternal
{
public:
    UIQT_PriorityQueueInternal();
    ~UIQT_PriorityQueueInternal();

    void Init();
    void Empty();

    bool Enqueue(TSharedPtr<FIQT_QueueItem>& InData);

    TSharedPtr<FIQT_QueueItem> Dequeue();

    bool Contains(TSharedPtr<FIQT_QueueItem>& InData);

    bool RemoveItem(TSharedPtr<FIQT_QueueItem>& ItemToRemove);

    bool ValidateData(const FIQT_QueueItem& InData) const;

    TSharedPtr<FIQT_QueueItem> FindByTaskID(const FGuid& TaskID);
    TSharedPtr<FIQT_QueueItem> FindByHashKey(FName InName, FGameplayTag InTag, bool bInIsOpen);

    void SetMaxSize(int32 NewSize);
    int32 GetMaxSize() const;

    int32 GetCount() const;
    int32 GetNumOpen() const;
    int32 GetNumClose() const;

    bool IsEmpty() const;

    void DumpVerificationList() const;

private:
    mutable FCriticalSection Mutex;         // Adicionado 'mutable' para permitir o uso em funções const
    int32 iQueueSize;               
    int32 iQueueMaxSize;            
    UIQT_DynAINode* pHead;         
    UIQT_DynAINode* pTail;         

    TArray<FIQT_QueueItem> VerificationList;

    void InsertNode(UIQT_DynAINode* InNode);
    void RemoveNode(UIQT_DynAINode* InNode);
    bool ValidateList() const;
};
