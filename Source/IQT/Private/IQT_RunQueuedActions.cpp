// IQT/Source/IQT/Private/IQT_RunQueuedActions.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "IQT_RunQueuedActions.h" 
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h" 
#include "AbilitySystemBlueprintLibrary.h" 
#include "Engine/World.h" 
#include "TimerManager.h" 
#include "AbilitySystemGlobals.h" // Para usar GetAbilitySystemComponentFromActor

UIQT_RunQueuedActions::UIQT_RunQueuedActions(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bOverallSuccess(true) 
{
    // Construtor: Inicializa o status de sucesso geral.
    // Tarefas de habilidade podem ser instanciadas por execu��o ou por ator.
    // 'Instanced Per Actor' � geralmente prefer�vel para gerenciadores de longo prazo.
}

UIQT_RunQueuedActions* UIQT_RunQueuedActions::IQT_RunQueuedActions(UGameplayAbility* OwningAbility, UIQT_Queue* InQueue)
{
    // Verifica se a habilidade propriet�ria e a fila s�o v�lidas.
    if (!OwningAbility || !InQueue)
    {
        UE_LOG(LogTemp, Error, TEXT("UIQT_RunQueuedActions: OwningAbility ou InQueue � nulo. N�o foi poss�vel criar a tarefa."));
        return nullptr;
    }

    // Cria uma nova inst�ncia desta Ability Task.
    UIQT_RunQueuedActions* MyObj = NewAbilityTask<UIQT_RunQueuedActions>(OwningAbility);
    MyObj->Queue = InQueue;
    MyObj->bOverallSuccess = true; // Reinicia o status de sucesso geral para esta execu��o.
    return MyObj;
}

void UIQT_RunQueuedActions::Activate()
{
    // Inicia o processamento do primeiro item da fila.
    // Esta � a entrada para o nosso loop ass�ncrono.
    ProcessNextQueueItem();
}

void UIQT_RunQueuedActions::ProcessNextQueueItem()
{
    // Limpa qualquer timer pendente para ProcessNextQueueItem, evitando chamadas duplicadas.
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(NextItemTimerHandle);
    }
    
    // Verifica se a fila � v�lida ou est� vazia.
    if (!Queue || Queue->IsQueueEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: Fila de a��es conclu�da. Sucesso geral: %s"), bOverallSuccess ? TEXT("TRUE") : TEXT("FALSE"));
        // Se a tarefa ainda deve enviar delegates (ou seja, n�o foi cancelada externamente), envia o status final.
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnFinished.Broadcast(bOverallSuccess); 
        }
        EndTask(); // Encerra a tarefa da habilidade.
        return;
    }

    FIQT_QueueItem CurrentItem;
    // Tenta desenfileirar um item.
    if (Queue->DequeueItem(CurrentItem))
    {
        // LOG 1: Confirma que o item foi desenfileirado e a a��o ser� disparada.
        UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: Desenfileirado item '%s'. Disparando a��o..."), *CurrentItem.Name.ToString());

        // Cria a tarefa de espera para a a��o atual.
        // O ator que ir� receber o evento de trigger � o OwnerActor da habilidade.
        CurrentWaitTask = UIQT_WaitForAction::IQT_WaitActionEvent(
            Ability, 
            CurrentItem, 
            Ability->GetActorInfo().OwnerActor.Get(), // O ator que ser� o alvo do evento de trigger
            true,  // OnlyTriggerOnce: A tarefa de espera termina ap�s o primeiro sucesso/falha.
            true  // OnlyMatchExact: Permite que tags filhas tamb�m acionem o evento.
        );
        
        // --- IN�CIO DE NOVOS LOGS E VERIFICA��ES DETALHADAS ---
        if (CurrentWaitTask)
        {
            // LOG 2: Confirma que a task UIQT_WaitForAction foi criada com sucesso.
            UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: UIQT_WaitForAction para item '%s' criada com sucesso. Validando TriggerData..."), *CurrentItem.Name.ToString());
            
            const FIQT_TriggerData& TriggerData = CurrentWaitTask->GetEventTriggerData();

            // Verifica��o 1: Garante que a TriggerData b�sica � v�lida.
            // Isso inclui verificar se a TriggerTag � v�lida e se o EventTargetActor existe e � v�lido.
            if (!TriggerData.TriggerTag.IsValid() || !IsValid(TriggerData.EventTargetActor))
            {
                // LOG 3: Indica que a TriggerData � inv�lida.
                UE_LOG(LogTemp, Warning, TEXT("UIQT_RunQueuedActions: TriggerData inv�lida para item '%s'. TriggerTag V�lida: %d, EventTargetActor V�lido: %d"),
                    *CurrentItem.Name.ToString(), TriggerData.TriggerTag.IsValid(), IsValid(TriggerData.EventTargetActor));
                
                bOverallSuccess = false; // Marca o resultado geral como falha.
                // Tenta agendar o processamento do pr�ximo item, j� que este n�o pode ser disparado.
                if (IsValid(Ability) && GetWorld()) 
                {
                    GetWorld()->GetTimerManager().SetTimer(NextItemTimerHandle, this, &UIQT_RunQueuedActions::ProcessNextQueueItem, 0.001f, false);
                }
                else
                {
                    EndTask(); // Se a Ability ou o mundo n�o s�o v�lidos, encerra a tarefa.
                }
                return; // Importante para sair da fun��o e evitar processamento adicional com dados inv�lidos.
            }

            // Verifica��o 2: Garante que o ator alvo possui um AbilitySystemComponent, pois � necess�rio para SendGameplayEventToActor.
            UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TriggerData.EventTargetActor);
            if (!TargetASC)
            {
                // LOG 4: Indica que o EventTargetActor n�o possui ASC.
                UE_LOG(LogTemp, Error, TEXT("UIQT_RunQueuedActions: EventTargetActor '%s' para item '%s' n�o possui AbilitySystemComponent. Evento de gameplay N�O SER� ENVIADO."),
                    *TriggerData.EventTargetActor->GetName(), *CurrentItem.Name.ToString());
                bOverallSuccess = false; // Marca o resultado geral como falha.
                // Tenta agendar o processamento do pr�ximo item, j� que este n�o pode ser disparado.
                if (IsValid(Ability) && GetWorld()) 
                {
                    GetWorld()->GetTimerManager().SetTimer(NextItemTimerHandle, this, &UIQT_RunQueuedActions::ProcessNextQueueItem, 0.001f, false);
                }
                else
                {
                    EndTask(); // Se a Ability ou o mundo n�o s�o v�lidos, encerra a tarefa.
                }
                return; // Importante para sair da fun��o e evitar tentar enviar evento para ASC nulo.
            }

            // Neste ponto, TriggerData (incluindo TriggerTag e EventTargetActor) e TargetASC devem ser v�lidos.
            // LOG 5: Confirma que o evento de gameplay ser� enviado.
            UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: Enviando evento '%s' para ator '%s' (ASC: %s)."), 
                *TriggerData.TriggerTag.ToString(), *TriggerData.EventTargetActor->GetName(), *TargetASC->GetName());
            
            // Dispara o evento de gameplay para o ator alvo especificado na TriggerData.
            UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TriggerData.EventTargetActor, TriggerData.TriggerTag, TriggerData.Payload);
            
            // LOG 6: Confirma a vincula��o dos delegates da task de espera.
            UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: Vinculando delegates para UIQT_WaitForAction de item '%s'."), *CurrentItem.Name.ToString());
            CurrentWaitTask->SucessesfullAction.AddDynamic(this, &UIQT_RunQueuedActions::OnActionSucceeded);
            CurrentWaitTask->FailedAction.AddDynamic(this, &UIQT_RunQueuedActions::OnActionFailed);
            
            // LOG 7: Confirma a ativa��o da task de espera.
            UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: Ativando UIQT_WaitForAction para item '%s'."), *CurrentItem.Name.ToString());
            CurrentWaitTask->Activate(); 
        }
        else // A cria��o da UIQT_WaitForAction falhou (j� logado internamente por IQT_WaitActionEvent se a falha for l�)
        {
            // LOG 8: Indica falha na cria��o da task UIQT_WaitForAction.
            UE_LOG(LogTemp, Error, TEXT("UIQT_RunQueuedActions: Falha ao criar UIQT_WaitForAction para item '%s'. Processando pr�ximo item."), *CurrentItem.Name.ToString());
            bOverallSuccess = false; 
            if (IsValid(Ability) && GetWorld()) 
            {
                GetWorld()->GetTimerManager().SetTimer(NextItemTimerHandle, this, &UIQT_RunQueuedActions::ProcessNextQueueItem, 0.001f, false);
            }
            else
            {
                EndTask();
            }
        }
    }
    else // DequeueItem falhou inesperadamente (a fila n�o estava vazia, mas DequeueItem retornou false)
    {
        UE_LOG(LogTemp, Error, TEXT("UIQT_RunQueuedActions: DequeueItem falhou inesperadamente. Encerrando tarefa com falha."));
        bOverallSuccess = false; 
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnFinished.Broadcast(bOverallSuccess);
        }
        EndTask();
    }
}

// Callback para quando uma a��o individual � conclu�da com sucesso.
void UIQT_RunQueuedActions::OnActionSucceeded(FGameplayEventData Payload, const FIQT_QueueItem& QueueItemData, FGameplayTag EventTag, AActor* EventTargetActor, FGameplayTag TriggerTag)
{
    UE_LOG(LogTemp, Log, TEXT("UIQT_RunQueuedActions: A��o '%s' conclu�da com SUCESSO (Tag: %s | TriggerTag: %s)."), *QueueItemData.Name.ToString(), *EventTag.ToString(), *TriggerTag.ToString());
    
    // Finaliza a CurrentWaitTask antes de agendar a pr�xima itera��o.
    // Isso � crucial para liberar recursos e desvincular delegates.
    if (CurrentWaitTask)
    {
        CurrentWaitTask->EndTask();
        CurrentWaitTask = nullptr; 
    }

    // Se a Ability que possui esta task ainda � v�lida e o mundo existe, agende o processamento do pr�ximo item.
    if (IsValid(Ability) && GetWorld()) 
    {
        GetWorld()->GetTimerManager().SetTimer(NextItemTimerHandle, this, &UIQT_RunQueuedActions::ProcessNextQueueItem, 0.001f, false);
    }
    else
    {
        EndTask(); // Se a Ability ou o mundo n�o s�o mais v�lidos, encerra esta tarefa.
    }
}

// Callback para quando uma a��o individual falha.
void UIQT_RunQueuedActions::OnActionFailed(FGameplayEventData Payload, const FIQT_QueueItem& QueueItemData, FGameplayTag EventTag, AActor* EventTargetActor, FGameplayTag TriggerTag)
{
    UE_LOG(LogTemp, Warning, TEXT("UIQT_RunQueuedActions: A��o '%s' falhou (Tag: %s | TriggerTag: %s)."), *QueueItemData.Name.ToString(), *EventTag.ToString(), *TriggerTag.ToString());
    bOverallSuccess = false; // Uma ao individual falhou, ent�o o sucesso geral � falso.
    
    // Finaliza a CurrentWaitTask antes de agendar a pr�xima itera��o.
    // Isso � crucial para liberar recursos e desvincular delegates.
    if (CurrentWaitTask)
    {
        CurrentWaitTask->EndTask();
        CurrentWaitTask = nullptr; 
    }

    // Se a Ability que possui esta task ainda � v�lida e o mundo existe, agende o processamento do pr�ximo item.
    if (IsValid(Ability) && GetWorld()) 
    {
        GetWorld()->GetTimerManager().SetTimer(NextItemTimerHandle, this, &UIQT_RunQueuedActions::ProcessNextQueueItem, 0.001f, false);
    }
    else
    {
        EndTask(); // Se a Ability ou o mundo n�o s�o mais v�lidos, encerra esta tarefa.
    }
}

// Chamado quando a Ability Task � destru�da.
void UIQT_RunQueuedActions::OnDestroy(bool AbilityEnding)
{
    // Limpa qualquer timer pendente para evitar chamadas ap�s a destrui��o da task.
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(NextItemTimerHandle);
    }
    // Garante que a tarefa de espera atual (CurrentWaitTask) seja limpa, se ainda ativa.
    if (CurrentWaitTask)
    {
        CurrentWaitTask->EndTask();
        CurrentWaitTask = nullptr;
    }
    // Chama a implementa��o base.
    Super::OnDestroy(AbilityEnding);
}
