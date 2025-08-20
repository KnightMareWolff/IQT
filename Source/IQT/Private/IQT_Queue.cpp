// IQT/Source/IQT/Private/IQT_Queue.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "IQT_Queue.h" 
#include "Internal/IQT_PriorityQueueInternal.h" 
// NOTA: "Internal/IQT_PriorityQueueInternal.h" AGORA É INCLUÍDO DIRETAMENTE EM "IQT_Queue.h" para resolver o TUniquePtr
// A linha abaixo foi comentada pois o include já está no .h do UIQT_Queue.
// #include "Internal/IQT_PriorityQueueInternal.h" 
DEFINE_LOG_CATEGORY(LogIOTQueue);



UIQT_Queue::UIQT_Queue()
    : EnqueueMode(EIQT_QueueMode::PriorityOrder) 
    , bIgnoreDuplicatesOnEnqueue(true)          
    , MaxQueueSize(0)                           
    , NextFIFOPriorityCounter(0)                
    , NextFILOPriorityCounter(TNumericLimits<int32>::Max()) 
{
    // A alocação da InternalQueue ainda ocorre aqui, mas a definição completa
    // de UIQT_PriorityQueueInternal já é conhecida devido ao include em IQT_Queue.h
    InternalQueue = MakeUnique<UIQT_PriorityQueueInternal>();
    InternalQueue->Init(); 
}

// Definição explícita do destrutor para resolver o erro C4150 (TUniquePtr incomplete type)
// A implementação vazia é suficiente, pois o TUniquePtr gerencia a memória.
UIQT_Queue::~UIQT_Queue() 
{
    // O TUniquePtr cuidará da desalocação da InternalQueue automaticamente
    // quando o UIQT_Queue for destruído.
    // Não é necessário chamar InternalQueue.Reset() aqui, a menos que haja
    // uma lógica específica de desalocação antecipada que precise ser executada
    // antes de BeginDestroy().
}

void UIQT_Queue::BeginDestroy()
{
    InternalQueue.Reset(); 
    Super::BeginDestroy();
}

void UIQT_Queue::InitializeQueue()
{
    if (InternalQueue.IsValid())
    {
        InternalQueue->Init(); 
        NextFIFOPriorityCounter = 0;
        NextFILOPriorityCounter = TNumericLimits<int32>::Max();
        UE_LOG(LogIOTQueue, Log, TEXT("UIQT_Queue: Fila inicializada e contadores resetados."));
    }
}

bool UIQT_Queue::EnqueueItem(FIQT_QueueItem& ItemToEnqueue)
{
    if (!InternalQueue.IsValid())
    {
        UE_LOG(LogIOTQueue, Error, TEXT("UIQT_Queue: Fila não inicializada! Chame InitializeQueue primeiro."));
        return false;
    }

    if (MaxQueueSize > 0 && InternalQueue->GetCount() >= MaxQueueSize)
    {
        UE_LOG(LogIOTQueue, Warning, TEXT("UIQT_Queue: Fila cheia (Max: %d). Item '%s' não enfileirado."), MaxQueueSize, *ItemToEnqueue.Name.ToString());
        return false;
    }

    if (bIgnoreDuplicatesOnEnqueue && ContainsItem(ItemToEnqueue))
    {
        UE_LOG(LogIOTQueue, Log, TEXT("UIQT_Queue: Item '%s' já existe na fila e duplicatas são ignoradas."), *ItemToEnqueue.Name.ToString());
        return false;
    }

    switch (EnqueueMode)
    {
        case EIQT_QueueMode::FIFO:
            ItemToEnqueue.Priority = NextFIFOPriorityCounter++;
            break;
        case EIQT_QueueMode::FILO:
            ItemToEnqueue.Priority = NextFILOPriorityCounter--;
            break;
        case EIQT_QueueMode::PriorityOrder:
        default:
            break;
    }

    TSharedPtr<FIQT_QueueItem> ItemPtr = MakeShared<FIQT_QueueItem>(ItemToEnqueue);
    ItemPtr->bIsEnqueued = true; 
    
    bool bSuccess = InternalQueue->Enqueue(ItemPtr);
    if (bSuccess)
    {
        UE_LOG(LogIOTQueue, Log , TEXT("UIQT_Queue: Enfileirado item '%s' com prioridade %d. Modo: %s."),
            *ItemToEnqueue.Name.ToString(), ItemToEnqueue.Priority,
            *UEnum::GetValueAsString(EnqueueMode));
    }
    return bSuccess;
}

bool UIQT_Queue::DequeueItem(FIQT_QueueItem& OutItem)
{
    if (!InternalQueue.IsValid())
    {
        UE_LOG(LogIOTQueue, Error, TEXT("UIQT_Queue: Fila não inicializada!"));
        return false;
    }

    TSharedPtr<FIQT_QueueItem> DequeuedPtr = InternalQueue->Dequeue();
    if (DequeuedPtr.IsValid())
    {
        OutItem = *DequeuedPtr; 
        OutItem.bIsEnqueued = false; 
        UE_LOG(LogIOTQueue, Log , TEXT("UIQT_Queue: Desenfileirado item '%s' com prioridade %d."),
            *OutItem.Name.ToString(), OutItem.Priority);
        return true;
    }

    UE_LOG(LogIOTQueue, Warning , TEXT("UIQT_Queue: Dequeued Pointer Invalido ou Lista Vazia, Item Vazio Retornado!"));
    OutItem = FIQT_QueueItem(); 
    return false;
}

bool UIQT_Queue::RemoveSpecificItem(FIQT_QueueItem& ItemToRemove)
{
    if (!InternalQueue.IsValid())
    {
        UE_LOG(LogIOTQueue, Error, TEXT("UIQT_Queue: Fila não inicializada!"));
        return false;
    }
    TSharedPtr<FIQT_QueueItem> TempItemPtr = MakeShared<FIQT_QueueItem>(ItemToRemove);
    bool bSuccess = InternalQueue->RemoveItem(TempItemPtr);
    if (bSuccess)
    {
        UE_LOG(LogIOTQueue, Log, TEXT("UIQT_Queue: Item '%s' removido especificamente da fila."), *ItemToRemove.Name.ToString());
    }
    return bSuccess;
}

bool UIQT_Queue::ContainsItem(FIQT_QueueItem& ItemToCheck) const
{
    if (!InternalQueue.IsValid())
    {
        UE_LOG(LogIOTQueue, Error, TEXT("UIQT_Queue: Fila não inicializada!"));
        return false;
    }
    TSharedPtr<FIQT_QueueItem> TempItemPtr = MakeShared<FIQT_QueueItem>(ItemToCheck); 
    return InternalQueue->Contains(TempItemPtr);
}

int32 UIQT_Queue::GetQueueCount() const
{
    if (InternalQueue.IsValid())
    {
        return InternalQueue->GetCount();
    }
    return 0;
}

bool UIQT_Queue::IsQueueEmpty() const
{
    if (InternalQueue.IsValid())
    {
        return InternalQueue->IsEmpty();
    }
    return true;
}

void UIQT_Queue::EmptyQueue()
{
    if (InternalQueue.IsValid())
    {
        InternalQueue->Empty();
        NextFIFOPriorityCounter = 0;
        NextFILOPriorityCounter = TNumericLimits<int32>::Max();
        UE_LOG(LogIOTQueue, Log, TEXT("UIQT_Queue: Fila esvaziada."));
    }
}

int32 UIQT_Queue::GetNumOpenItems() const
{
    if (InternalQueue.IsValid())
    {
        return InternalQueue->GetNumOpen();
    }
    return 0;
}

int32 UIQT_Queue::GetNumClosedItems() const
{
    if (InternalQueue.IsValid())
    {
        return InternalQueue->GetNumClose();
    }
    return 0;
}

bool UIQT_Queue::ValidateQueueItemData(const FIQT_QueueItem& ItemToValidate) const
{
    if (InternalQueue.IsValid())
    {
        return InternalQueue->ValidateData(ItemToValidate);
    }
    return false; 
}

bool UIQT_Queue::FindItemByTaskID(const FGuid& TaskID, FIQT_QueueItem& OutItem) const
{
    if (!InternalQueue.IsValid())
    {
        UE_LOG(LogIOTQueue, Error, TEXT("UIQT_Queue: Fila não inicializada!"));
        OutItem = FIQT_QueueItem(); 
        return false;
    }
    TSharedPtr<FIQT_QueueItem> FoundItemPtr = InternalQueue->FindByTaskID(TaskID);
    if (FoundItemPtr.IsValid())
    {
        OutItem = *FoundItemPtr;
        return true;
    }
    OutItem = FIQT_QueueItem(); 
    return false;
}

bool UIQT_Queue::FindItemByHashKey(FName InName, FGameplayTag InTag, bool bInIsOpen, FIQT_QueueItem& OutItem) const
{
    if (!InternalQueue.IsValid())
    {
        UE_LOG(LogIOTQueue, Error, TEXT("UIQT_Queue: Fila não inicializada!"));
        OutItem = FIQT_QueueItem(); 
        return false;
    }
    TSharedPtr<FIQT_QueueItem> FoundItemPtr = InternalQueue->FindByHashKey(InName, InTag, bInIsOpen);
    if (FoundItemPtr.IsValid())
    {
        OutItem = *FoundItemPtr;
        return true;
    }
    OutItem = FIQT_QueueItem(); 
    return false;
}
