// IQT/Source/IQT/Private/Internal/IQT_DynAINode.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "IQT_DynAINode.h" 

UIQT_DynAINode::UIQT_DynAINode()
    : pNextNode(nullptr)
    , pPriorNode(nullptr)
    , pFather(nullptr)
    , AgentData(nullptr) 
{}

void UIQT_DynAINode::Init(TSharedPtr<FIQT_QueueItem>& InData)
{
    ResetNode();
    AgentData = InData; 
}

void UIQT_DynAINode::ResetNode()
{
    AgentData   = nullptr;
    pFather     = nullptr;
    pNextNode   = nullptr;
    pPriorNode  = nullptr;
}

void UIQT_DynAINode::SetPriority(int32 InPriority)
{
    if (AgentData.IsValid())
    {
        AgentData->Priority = InPriority;
    }
}

int32 UIQT_DynAINode::GetPriority() const
{
    if (AgentData.IsValid())
    {
        return AgentData->Priority;
    }
    return 0; 
}

bool UIQT_DynAINode::operator==(const UIQT_DynAINode& Other) const
{
    if (!AgentData.IsValid() || !Other.AgentData.IsValid())
    {
        return !AgentData.IsValid() && !Other.AgentData.IsValid();
    }
    return (*AgentData) == (*Other.AgentData); 
}

bool UIQT_DynAINode::operator<(const UIQT_DynAINode& Other) const
{
    if (!AgentData.IsValid() || !Other.AgentData.IsValid())
    {
        return AgentData.IsValid() && !Other.AgentData.IsValid();
    }
    return (*AgentData) < (*Other.AgentData); 
}

bool UIQT_DynAINode::operator>(const UIQT_DynAINode& Other) const
{
    if (!AgentData.IsValid() || !Other.AgentData.IsValid())
    {
        return !AgentData.IsValid() && Other.AgentData.IsValid();
    }
    return (*AgentData) > (*Other.AgentData); 
}
