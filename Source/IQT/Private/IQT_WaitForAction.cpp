// IQT/Source/IQT/Private/IQT_WaitForAction.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "IQT_WaitForAction.h" 
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h" // Para UAbilitySystemGlobals::GetAbilitySystemComponentFromActor
#include "Abilities/GameplayAbility.h" // Necessário para UGameplayAbility::GetActorInfo()
#include "GameFramework/Actor.h" // Para AActor

UIQT_WaitForAction::UIQT_WaitForAction(const FObjectInitializer& ObjectInitializer) 
    : Super(ObjectInitializer)
    , bUseExternalTarget(false)
    , bOnlyTriggerOnce(false)
    , bOnlyMatchExact(true)
{
    // Construtor, inicializa variáveis membro.
}

UIQT_WaitForAction* UIQT_WaitForAction::IQT_WaitActionEvent(UGameplayAbility* OwningAbility, 
    const FIQT_QueueItem& InQueueItem,
    AActor* InOptionalExternalTarget,
    bool InOnlyTriggerOnce,
    bool InOnlyMatchExact)
{
    // Verifica se a Ability de origem é válida.
    if (!OwningAbility)
    {
        UE_LOG(LogTemp, Error, TEXT("UIQT_WaitForAction: OwningAbility é nulo. Não foi possível criar a tarefa."));
        return nullptr;
    }

    // Cria uma nova instância da Ability Task.
    UIQT_WaitForAction* MyObj = NewAbilityTask<UIQT_WaitForAction>(OwningAbility); 
    
    // Armazena os dados do item da fila.
    MyObj->QueueItemData = InQueueItem;

    // Configura as tags de sucesso e falha A PARTIR DO ITEM DA FILA.
    MyObj->SuccessTag = InQueueItem.AbilityEndTag;
    MyObj->FailTag = InQueueItem.AbilityFailTag;

    // Configura os comportamentos da tarefa.
    MyObj->bOnlyTriggerOnce = InOnlyTriggerOnce;
    MyObj->bOnlyMatchExact = InOnlyMatchExact;

    // *** TRATAMENTO DO OptionalExternalTarget ***
    // Se um ator externo foi fornecido, tenta obter o ASC dele.
    AActor* CurrentOwnerActor = OwningAbility->GetActorInfo().OwnerActor.Get();

    if (InOptionalExternalTarget)
    {
        MyObj->bUseExternalTarget = true;
        MyObj->OptionalExternalTargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InOptionalExternalTarget);
        if (!MyObj->OptionalExternalTargetASC)
        {
            UE_LOG(LogTemp, Warning, TEXT("UIQT_WaitForAction: InOptionalExternalTarget (%s) não possui um AbilitySystemComponent válido. Usando OwningAbility's ASC como fallback."), *InOptionalExternalTarget->GetName());
            MyObj->bUseExternalTarget = false;
            MyObj->OptionalExternalTargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentOwnerActor);
        }
    }
    else
    {
        MyObj->OptionalExternalTargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentOwnerActor);
    }

    // *** PREENCHE A NOVA ESTRUTURA FIQT_TriggerData ***
    MyObj->TriggerDataToUse.TriggerTag = InQueueItem.AbilityTriggerTag;
    MyObj->TriggerDataToUse.Payload.EventTag = InQueueItem.AbilityTriggerTag;
    MyObj->TriggerDataToUse.Payload.Instigator = CurrentOwnerActor;
    MyObj->TriggerDataToUse.Payload.Target = CurrentOwnerActor;
    MyObj->TriggerDataToUse.Payload.OptionalObject = InQueueItem.UserPayload;
    MyObj->TriggerDataToUse.EventTargetActor = CurrentOwnerActor;


    // Retorna a tarefa recém-criada.
    return MyObj;
}

// Retorna TObjectPtr<UAbilitySystemComponent> para consistência
TObjectPtr<UAbilitySystemComponent> UIQT_WaitForAction::GetTargetASC() const 
{
    // Retorna o ASC externo se estiver em uso e válido, caso contrário, retorna o ASC do OwningAbility.
    if (bUseExternalTarget && OptionalExternalTargetASC)
    {
        return OptionalExternalTargetASC;
    }
    // Retorna o ASC da Ability que criou esta tarefa (padrão).
    return AbilitySystemComponent.Get(); 
}

void UIQT_WaitForAction::Activate() 
{
    TObjectPtr<UAbilitySystemComponent> TargetASC = GetTargetASC(); 
    // Se o ASC alvo não for válido, loga um aviso e encerra a tarefa.
    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("UIQT_WaitForAction: TargetASC é nulo, abortando tarefa.")); 
        EndTask();
        return;
    }

    // Se nenhuma tag de sucesso ou falha for fornecida, a tarefa não terá como disparar.
    // Loga um aviso e encerra a tarefa.
    if (!SuccessTag.IsValid() && !FailTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UIQT_WaitForAction: Nenhuma SuccessTag ou FailTag válida fornecida (do QueueItem), a tarefa nunca será disparada. Encerrando tarefa.")); 
        EndTask();
        return;
    }

    // Liga os delegates para a tag de sucesso.
    if (SuccessTag.IsValid())
    {
        if (bOnlyMatchExact)
        {
            // Para correspondência exata, usa GenericGameplayEventCallbacks.
            SuccessHandle = TargetASC->GenericGameplayEventCallbacks.FindOrAdd(SuccessTag).AddUObject(this, &UIQT_WaitForAction::OnExactSuccessEvent);
        }
        else
        {
            // Para correspondência de container (tags aninhadas), usa AddGameplayEventTagContainerDelegate.
            SuccessHandle = TargetASC->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(SuccessTag), FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UIQT_WaitForAction::OnContainerSuccessEvent));
        }
    }

    // Liga os delegates para a tag de falha.
    if (FailTag.IsValid())
    {
        if (bOnlyMatchExact)
        {
            // Para correspondência exata, usa GenericGameplayEventCallbacks.
            FailHandle = TargetASC->GenericGameplayEventCallbacks.FindOrAdd(FailTag).AddUObject(this, &UIQT_WaitForAction::OnExactFailEvent);
        }
        else
        {
            // Para correspondência de container (tags aninhadas), usa AddGameplayEventTagContainerDelegate.
            FailHandle = TargetASC->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(FailTag), FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UIQT_WaitForAction::OnContainerFailEvent));
        }
    }
}

// Callback para correspondência exata de sucesso.
void UIQT_WaitForAction::OnExactSuccessEvent(const FGameplayEventData* Payload) 
{
    // Verifica se os delegates devem ser broadcastados e se o payload é válido.
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        // Broadcasta o evento de sucesso, incluindo a TriggerTag original.
        SucessesfullAction.Broadcast(*Payload, QueueItemData, SuccessTag, GetTargetASC()->GetOwner(), QueueItemData.AbilityTriggerTag); 
        // Se a tarefa deve disparar apenas uma vez, a encerra.
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

// Callback para correspondência de container de sucesso.
void UIQT_WaitForAction::OnContainerSuccessEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload) 
{
    // Verifica se os delegates devem ser broadcastados e se o payload é válido.
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        // Broadcasta o evento de sucesso, incluindo a TriggerTag original.
        SucessesfullAction.Broadcast(*Payload, QueueItemData, MatchedTag, GetTargetASC()->GetOwner(), QueueItemData.AbilityTriggerTag); 
        // Se a tarefa deve disparar apenas uma vez, a encerra.
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

// Callback para correspondência exata de falha.
void UIQT_WaitForAction::OnExactFailEvent(const FGameplayEventData* Payload) 
{
    // Verifica se os delegates devem ser broadcastados e se o payload é válido.
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        // Broadcasta o evento de falha, incluindo a TriggerTag original.
        FailedAction.Broadcast(*Payload, QueueItemData, FailTag, GetTargetASC()->GetOwner(), QueueItemData.AbilityTriggerTag);
        // Se a tarefa deve disparar apenas uma vez, a encerra.
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

// Callback para correspondência de container de falha.
void UIQT_WaitForAction::OnContainerFailEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload) 
{
    // Verifica se os delegates devem ser broadcastados e se o payload é válido.
    if (ShouldBroadcastAbilityTaskDelegates() && Payload)
    {
        // Broadcasta o evento de falha, incluindo a TriggerTag original.
        FailedAction.Broadcast(*Payload, QueueItemData, MatchedTag, GetTargetASC()->GetOwner(), QueueItemData.AbilityTriggerTag);
        // Se a tarefa deve disparar apenas uma vez, a encerra.
        if (bOnlyTriggerOnce)
        {
            EndTask();
        }
    }
}

void UIQT_WaitForAction::OnDestroy(bool AbilityEnding) 
{
    TObjectPtr<UAbilitySystemComponent> TargetASC = GetTargetASC(); 
    
    // Remove os delegates para evitar vazamentos de memória e chamadas indesejadas.
    if (TargetASC)
    {
        if (SuccessHandle.IsValid())
        {
            if (bOnlyMatchExact)
            {
                TargetASC->GenericGameplayEventCallbacks.FindOrAdd(SuccessTag).Remove(SuccessHandle);
            }
            else
            {
                TargetASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(SuccessTag), SuccessHandle);
            }
        }

        if (FailHandle.IsValid())
        {
            if (bOnlyMatchExact)
            {
                TargetASC->GenericGameplayEventCallbacks.FindOrAdd(FailTag).Remove(FailHandle);
            }
            else
            {
                TargetASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(FailTag), FailHandle);
            }
        }
    }

    // Chama a implementação base.
    Super::OnDestroy(AbilityEnding);
}
