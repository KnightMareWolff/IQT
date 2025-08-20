// IQT/Source/IQT/Public/IQT_RunQueuedActions.h
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "IQT_Queue.h" 
#include "IQT_WaitForAction.h" 
#include "IQT_DataTypes.h" 
#include "Engine/World.h" // Necess�rio para FTimerHandle
#include "TimerManager.h" // Necess�rio para FTimerHandle

#include "IQT_RunQueuedActions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRunQueuedActionsFinishedDelegate, bool, bSuccess);

/**
 * UIQT_RunQueuedActions: Uma AbilityTask para executar todas as a��es em uma fila de forma sequencial.
 * Desenfileira cada item, dispara seu evento de trigger e espera pela conclus�o usando UIQT_WaitForAction.
 */
UCLASS()
class IQT_API UIQT_RunQueuedActions : public UAbilityTask
{
    GENERATED_BODY()

public:
    UIQT_RunQueuedActions(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintAssignable)
    FRunQueuedActionsFinishedDelegate OnFinished; 

    /**
     * Inicia a execu��o sequencial das a��es em uma fila.
     * @param OwningAbility A habilidade que est� iniciando esta tarefa.
     * @param InQueue O componente UIQT_Queue a ser processado.
     * @return Uma inst�ncia da tarefa UIQT_RunQueuedActions.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT|Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UIQT_RunQueuedActions* IQT_RunQueuedActions(UGameplayAbility* OwningAbility, UIQT_Queue* InQueue);

    virtual void Activate() override;
    virtual void OnDestroy(bool AbilityEnding) override;

protected:
    UPROPERTY()
    TObjectPtr<UIQT_Queue> Queue; 

    UPROPERTY()
    TObjectPtr<UIQT_WaitForAction> CurrentWaitTask; 

    bool bOverallSuccess; 

    // Handle para agendar a pr�xima chamada de ProcessNextQueueItem
    FTimerHandle NextItemTimerHandle;

    /**
     * Processa o pr�ximo item da fila de forma recursiva (agendada).
     */
    void ProcessNextQueueItem();

    /**
     * Callback quando UIQT_WaitForAction para um item individual � bem-sucedida.
     */
    UFUNCTION()
    void OnActionSucceeded(FGameplayEventData Payload, const FIQT_QueueItem& QueueItemData, FGameplayTag EventTag, AActor* EventTargetActor, FGameplayTag TriggerTag);

    /**
     * Callback quando UIQT_WaitForAction para um item individual falha.
     */
    UFUNCTION()
    void OnActionFailed(FGameplayEventData Payload, const FIQT_QueueItem& QueueItemData, FGameplayTag EventTag, AActor* EventTargetActor, FGameplayTag TriggerTag);
};
