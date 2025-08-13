// IQT/Source/IQT/Private/Internal/IQT_DynAINode.h
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "IQT_DataTypes.h" // Incluindo FIQT_DataTypes.h

/**
 * UIQT_DynAINode: Representa um nó na lista duplamente encadeada da fila de prioridade.
 * Contém um TSharedPtr para os dados do item da fila (FIQT_QueueItem) e ponteiros para os nós vizinhos.
 */
class UIQT_DynAINode
{
public:
    UIQT_DynAINode();

    UIQT_DynAINode(const UIQT_DynAINode& Other) = default;
    UIQT_DynAINode& operator=(const UIQT_DynAINode& Other) = default;

    void Init(TSharedPtr<FIQT_QueueItem>& InData);

    void ResetNode();

    void SetPriority(int32 InPriority);
    int32 GetPriority() const;

    bool operator==(const UIQT_DynAINode& Other) const;
    bool operator<(const UIQT_DynAINode& Other) const;
    bool operator>(const UIQT_DynAINode& Other) const;

public:
    UIQT_DynAINode* pNextNode;  
    UIQT_DynAINode* pPriorNode; 
    UIQT_DynAINode* pFather;    

    TSharedPtr<FIQT_QueueItem> AgentData;
};

