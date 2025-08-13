// IQT/Source/IQT/Private/IQT_WaitForAction.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "IQT_WaitForAction.h" 
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h" // Para UAbilitySystemGlobals::GetAbilitySystemComponentFromActor
#include "Abilities/GameplayAbility.h"

UIQT_WaitForAction::UIQT_WaitForAction(const FObjectInitializer& ObjectInitializer) 
    : Super(ObjectInitializer)
    , bUseExternalTarget(false)
    , bOnlyTriggerOnce(false)
    , bOnlyMatchExact(true)
{
    // Constructor logic, if any.
}

UIQT_WaitForAction* UIQT_WaitForAction::IQT_WaitActionEvent(UGameplayAbility* OwningAbility, 
    FGameplayTag InSuccessTag,
    FGameplayTag InFailTag,
    AActor* InOptionalExternalTarget,
    bool InOnlyTriggerOnce,
    bool InOnlyMatchExact)
{
    UIQT_WaitForAction* MyObj = NewAbilityTask<UIQT_WaitForAction>(OwningAbility); 

    MyObj->SuccessTag = InSuccessTag;
    MyObj->FailTag = InFailTag;
    MyObj->bOnlyTriggerOnce = InOnlyTriggerOnce;
    MyObj->bOnlyMatchExact = InOnlyMatchExact;

    if (InOptionalExternalTarget)
    {
        MyObj->bUseExternalTarget = true;
        MyObj->OptionalExternalTargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InOptionalExternalTarget);
    }

    return MyObj;
}

UAbilitySystemComponent* UIQT_WaitForAction::GetTargetASC() const 
{
    if (bUseExternalTarget && OptionalExternalTargetASC.Get()) 
    {
        return OptionalExternalTargetASC.Get();
    }
    return AbilitySystemComponent.Get(); // Use ASC do OwningAbility por padr�o
}

void UIQT_WaitForAction::Activate() 
{
    UAbilitySystemComponent* TargetASC = GetTargetASC();
    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("UIQT_WaitForAction: TargetASC is null, aborting task.")); 
        EndTask();
        return;
    }

    if (SuccessTag.IsValid())
    {
        if (bOnlyMatchExact)
        {
            // CORRE��O: GenericGameplayEventCallbacks com OnExactSuccessEvent (FGameplayEventData*)
            SuccessHandle = TargetASC->GenericGameplayEventCallbacks.FindOrAdd(SuccessTag).AddUObject(this, &UIQT_WaitForAction::OnExactSuccessEvent);
        }
        else
        {
            // CORRE��O: AddGameplayEventTagContainerDelegate com OnContainerSuccessEvent (FGameplayTag, FGameplayEventData*)
            SuccessHandle = TargetASC->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(SuccessTag), FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UIQT_WaitForAction::OnContainerSuccessEvent));
        }
    }

    if (FailTag.IsValid())
    {
        if (bOnlyMatchExact)
        {
            // CORRE��O: GenericGameplayEventCallbacks com OnExactFailEvent (FGameplayEventData*)
            FailHandle = TargetASC->GenericGameplayEventCallbacks.FindOrAdd(FailTag).AddUObject(this, &UIQT_WaitForAction::OnExactFailEvent);
        }
        else
        {
            // CORRE��O: AddGameplayEventTagContainerDelegate com OnContainerFailEvent (FGameplayTag, FGameplayEventData*)
            FailHandle = TargetASC->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(FailTag), FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UIQT_WaitForAction::OnContainerFailEvent));
        }
    }

    if (!SuccessTag.IsValid() && !FailTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UIQT_WaitForAction: No valid SuccessTag or FailTag provided, task will never trigger. Ending task.")); 
        EndTask();
    }
}

// Implementa��es das novas fun��es de callback

// Callback para correspond�ncia exata
void UIQT_WaitForAction::OnExactSuccessEvent(const FGameplayEventData* Payload) 
{
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        // O payload j� vem como ponteiro, ent�o dereferenciamos para o Broadcast que espera por valor
        SucessesfullAction.Broadcast(*Payload); 
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

// Callback para correspond�ncia de container
void UIQT_WaitForAction::OnContainerSuccessEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload) 
{
    // A tag MatchedTag � a que efetivamente disparou o evento.
    // Verificamos o Payload para garantir que n�o � nulo antes de usar.
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        // O payload j� vem como ponteiro, ent�o dereferenciamos para o Broadcast que espera por valor
        SucessesfullAction.Broadcast(*Payload); 
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

// Callback para correspond�ncia exata (falha)
void UIQT_WaitForAction::OnExactFailEvent(const FGameplayEventData* Payload) 
{
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        FailedAction.Broadcast(*Payload);
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

// Callback para correspond�ncia de container (falha)
void UIQT_WaitForAction::OnContainerFailEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload) 
{
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        FailedAction.Broadcast(*Payload);
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

void UIQT_WaitForAction::OnDestroy(bool AbilityEnding) 
{
    UAbilitySystemComponent* TargetASC = GetTargetASC();
    
    // CORRE��O: L�gica de remo��o de delegates baseada no modo de binding
    if (TargetASC && SuccessHandle.IsValid())
    {
        if (bOnlyMatchExact)
        {
            // Remover para GenericGameplayEventCallbacks
            TargetASC->GenericGameplayEventCallbacks.FindOrAdd(SuccessTag).Remove(SuccessHandle);
        }
        else
        {
            // Remover para AddGameplayEventTagContainerDelegate
            TargetASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(SuccessTag), SuccessHandle);
        }
    }

    if (TargetASC && FailHandle.IsValid())
    {
        if (bOnlyMatchExact)
        {
            // Remover para GenericGameplayEventCallbacks
            TargetASC->GenericGameplayEventCallbacks.FindOrAdd(FailTag).Remove(FailHandle);
        }
        else
        {
            // Remover para AddGameplayEventTagContainerDelegate
            TargetASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(FailTag), FailHandle);
        }
    }

    Super::OnDestroy(AbilityEnding);
}
