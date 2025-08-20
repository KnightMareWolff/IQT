// IQT/Source/IQT/Private/Internal/IQT_PriorityQueueInternal.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "IQT_PriorityQueueInternal.h" 
#include "IQT_DynAINode.h" 
#include "Kismet/GameplayStatics.h" 

// Construtor
UIQT_PriorityQueueInternal::UIQT_PriorityQueueInternal()
    : iQueueSize(0)
    , iQueueMaxSize(300) 
{
    pHead = new UIQT_DynAINode();
    pTail = new UIQT_DynAINode();

    pHead->pNextNode = pTail;
    pTail->pPriorNode = pHead;
}

// Destrutor
UIQT_PriorityQueueInternal::~UIQT_PriorityQueueInternal()
{
    FScopeLock Lock(&Mutex); 
    Empty();                 
    delete pHead;            
    delete pTail;
    pHead = nullptr;
    pTail = nullptr;
}

void UIQT_PriorityQueueInternal::Init()
{
    FScopeLock Lock(&Mutex); 
    Empty(); 

    if (!pHead) pHead = new UIQT_DynAINode();
    if (!pTail) pTail = new UIQT_DynAINode();

    pHead->pNextNode = pTail;
    pTail->pPriorNode = pHead;
    iQueueSize = 0;
    // VerificationList.Empty(); // Removido
}

void UIQT_PriorityQueueInternal::Empty()
{
    FScopeLock Lock(&Mutex); 
    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        UIQT_DynAINode* Next = Current->pNextNode;
        delete Current; 
        Current = Next;
    }

    pHead->pNextNode = pTail;
    pTail->pPriorNode = pHead;
    iQueueSize = 0;
    // VerificationList.Empty(); // Removido
}

bool UIQT_PriorityQueueInternal::Enqueue(TSharedPtr<FIQT_QueueItem>& InData)
{
    FScopeLock Lock(&Mutex); 

    if (!InData.IsValid() || !ValidateData(*InData))
    {
        UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal: Tentativa de enfileirar dados inválidos ou nulos."));
        return false;
    }

    // REMOVIDO: A checagem de duplicidade agora é responsabilidade exclusiva do UIQT_Queue.
    // if (Contains(InData))
    // {
    //     return false; 
    // }

    if (iQueueSize >= iQueueMaxSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal: Fila atingiu o tamanho máximo (%d). Item '%s' não enfileirado."), iQueueMaxSize, *InData->Name.ToString());
        return false; 
    }

    UIQT_DynAINode* NewNode = new UIQT_DynAINode();
    NewNode->Init(InData); 

    InsertNode(NewNode); 

    // REMOVIDO: Não há mais VerificationList.
    // VerificationList.AddUnique(*InData); 

    iQueueSize++;
    return true;
}

TSharedPtr<FIQT_QueueItem> UIQT_PriorityQueueInternal::Dequeue()
{
    FScopeLock Lock(&Mutex); 
    if (iQueueSize == 0)
    {
        return nullptr; 
    }

    UIQT_DynAINode* NodeToRemove = pHead->pNextNode; 
    TSharedPtr<FIQT_QueueItem> DequeuedData = NodeToRemove->AgentData; 

    NodeToRemove->pPriorNode->pNextNode = NodeToRemove->pNextNode;
    NodeToRemove->pNextNode->pPriorNode = NodeToRemove->pPriorNode;

    delete NodeToRemove; 
    NodeToRemove = nullptr; 

    iQueueSize--;

    // REMOVIDO: Não há mais VerificationList para atualizar.
    // if (DequeuedData.IsValid())
    // {
    //     VerificationList.RemoveAll([&](const FIQT_QueueItem& Item) {
    //         return Item == *DequeuedData; 
    //     });
    // }

    return DequeuedData;
}

void UIQT_PriorityQueueInternal::InsertNode(UIQT_DynAINode* InNode)
{
    UIQT_DynAINode* Current = pHead;
    while (Current->pNextNode != pTail && InNode->GetPriority() > Current->pNextNode->GetPriority())
    {
        Current = Current->pNextNode;
    }

    InNode->pNextNode = Current->pNextNode;
    InNode->pPriorNode = Current;
    Current->pNextNode->pPriorNode = InNode;
    Current->pNextNode = InNode;
}

bool UIQT_PriorityQueueInternal::Contains(TSharedPtr<FIQT_QueueItem>& InData)
{
    FScopeLock Lock(&Mutex); 
    if (!InData.IsValid()) return false;

    // NOVO: Itera a lista encadeada diretamente para verificar a existência.
    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && (*Current->AgentData) == (*InData))
        {
            return true;
        }
        Current = Current->pNextNode;
    }
    return false;
}

bool UIQT_PriorityQueueInternal::RemoveItem(TSharedPtr<FIQT_QueueItem>& ItemToRemove)
{
    FScopeLock Lock(&Mutex); 
    if (!ItemToRemove.IsValid()) return false;

    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && (*Current->AgentData) == (*ItemToRemove))
        {
            RemoveNode(Current);
            iQueueSize--;

            // REMOVIDO: Não há mais VerificationList para atualizar.
            // VerificationList.RemoveAll([&](const FIQT_QueueItem& Item) {
            //     return Item == *ItemToRemove;
            // });

            return true;
        }
        Current = Current->pNextNode;
    }
    return false; 
}

void UIQT_PriorityQueueInternal::RemoveNode(UIQT_DynAINode* InNode)
{
    if (!InNode || InNode == pHead || InNode == pTail)
    {
        return; 
    }

    InNode->pPriorNode->pNextNode = InNode->pNextNode;
    InNode->pNextNode->pPriorNode = InNode->pPriorNode;

    delete InNode;
}

bool UIQT_PriorityQueueInternal::ValidateData(const FIQT_QueueItem& InData) const
{
    if (InData.Name.IsNone()) return false;
    if (!InData.AbilityTriggerTag.IsValid()) return false;
    
    return true;
}

TSharedPtr<FIQT_QueueItem> UIQT_PriorityQueueInternal::FindByTaskID(const FGuid& TaskID)
{
    FScopeLock Lock(&Mutex); 
    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && Current->AgentData->TaskID == TaskID)
        {
            return Current->AgentData;
        }
        Current = Current->pNextNode;
    }
    return nullptr;
}

TSharedPtr<FIQT_QueueItem> UIQT_PriorityQueueInternal::FindByHashKey(FName InName, FGameplayTag InTag, bool bInIsOpen)
{
    FScopeLock Lock(&Mutex); 
    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && 
            Current->AgentData->Name == InName &&
            Current->AgentData->AbilityTriggerTag.MatchesTagExact(InTag) &&
            Current->AgentData->bIsOpen == bInIsOpen)
        {
            return Current->AgentData;
        }
        Current = Current->pNextNode;
    }
    return nullptr;
}

void UIQT_PriorityQueueInternal::SetMaxSize(int32 NewSize)
{
    FScopeLock Lock(&Mutex); 
    if (NewSize > 0)
    {
        iQueueMaxSize = NewSize;
    }
}

int32 UIQT_PriorityQueueInternal::GetMaxSize() const
{
    FScopeLock Lock(&Mutex); 
    return iQueueMaxSize;
}

int32 UIQT_PriorityQueueInternal::GetCount() const
{
    FScopeLock Lock(&Mutex); 
    return iQueueSize;
}

int32 UIQT_PriorityQueueInternal::GetNumOpen() const
{
    FScopeLock Lock(&Mutex); 
    int32 Count = 0;
    // NOVO: Itera a lista encadeada diretamente para contar itens abertos.
    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && Current->AgentData->bIsOpen)
        {
            Count++;
        }
        Current = Current->pNextNode;
    }
    return Count;
}

int32 UIQT_PriorityQueueInternal::GetNumClose() const
{
    FScopeLock Lock(&Mutex); 
    int32 Count = 0;
    // NOVO: Itera a lista encadeada diretamente para contar itens fechados.
    UIQT_DynAINode* Current = pHead->pNextNode;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && !Current->AgentData->bIsOpen)
        {
            Count++;
        }
        Current = Current->pNextNode;
    }
    return Count;
}

bool UIQT_PriorityQueueInternal::IsEmpty() const
{
    FScopeLock Lock(&Mutex); 
    return iQueueSize == 0;
}

// RENOMEADO e MODIFICADO para despejar o conteúdo real da fila, não uma lista de verificação separada.
void UIQT_PriorityQueueInternal::DumpQueueContents() const
{
    FScopeLock Lock(&Mutex); 
    UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal - Conteúdo Atual da Fila"));
    UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal - ============================================================="));

    UIQT_DynAINode* Current = pHead->pNextNode;
    int32 Index = 0;
    while (Current != pTail)
    {
        if (Current->AgentData.IsValid() && ValidateData(*Current->AgentData))
        {
            UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal - Item %d: Name=%s | Tag=%s | IsOpen=%s | Priority=%d | TaskID=%s"),
                Index, *Current->AgentData->Name.ToString(), *Current->AgentData->AbilityTriggerTag.ToString(), 
                Current->AgentData->bIsOpen ? TEXT("true") : TEXT("false"), Current->AgentData->Priority, 
                *Current->AgentData->TaskID.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal - Item %d: Dados inválidos no nó."), Index);
        }
        Current = Current->pNextNode;
        Index++;
    }
    UE_LOG(LogTemp, Warning, TEXT("UIQT_PriorityQueueInternal - ============================================================="));
}

bool UIQT_PriorityQueueInternal::ValidateList() const
{
    FScopeLock Lock(&Mutex); 
    UIQT_DynAINode* Current = pHead;
    int32 Count = 0;
    while (Current != nullptr)
    {
        if (Current->pNextNode == nullptr && Current != pTail)
        {
            UE_LOG(LogTemp, Error, TEXT("UIQT_PriorityQueueInternal: Erro de validação da lista: Nó '%p' sem pNextNode antes do pTail."), Current);
            return false;
        }
        if (Current->pNextNode && Current->pNextNode->pPriorNode != Current)
        {
            UE_LOG(LogTemp, Error, TEXT("UIQT_PriorityQueueInternal: Erro de validação da lista: Inconsistência de ponteiro duplo. Current->pNextNode->pPriorNode != Current."));
            return false;
        }
        if (Current != pHead && Current != pTail)
        {
            Count++; 
        }
        Current = Current->pNextNode;
    }
    if (Count != iQueueSize)
    {
        UE_LOG(LogTemp, Error, TEXT("UIQT_PriorityQueueInternal: Erro de validação da lista: Contagem de nós difere do iQueueSize. Contado: %d, Esperado: %d"), Count, iQueueSize);
        return false;
    }
    return true;
}