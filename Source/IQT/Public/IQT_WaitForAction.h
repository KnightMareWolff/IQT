// IQT/Source/IQT/Public/IQT_WaitForAction.h
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "UObject/ObjectMacros.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h" 
#include "IQT_DataTypes.h" // Inclui FIQT_QueueItem E a nova FIQT_TriggerData

#include "IQT_WaitForAction.generated.h" 

class UAbilitySystemComponent;
class AActor; // Forward declaration para AActor

// Delegates agora incluem FIQT_QueueItem, a Tag do evento (a que matched), o Ator Alvo e a TriggerTag original.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FWaitActionEventDelegate, FGameplayEventData, Payload, const FIQT_QueueItem&, QueueItemData, FGameplayTag, EventTag, AActor*, EventTargetActor, FGameplayTag, TriggerTag);

/**
 * UIQT_WaitForAction: Uma AbilityTask que espera por eventos de Gameplay Tag.
 * Permite que uma Gameplay Ability pause sua execução até que uma tag de sucesso ou falha seja emitida.
 * Atualizado para retornar diretamente o FIQT_QueueItem, a tag que disparou o evento e o ator alvo.
 */
UCLASS()
class IQT_API UIQT_WaitForAction : public UAbilityTask 
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FWaitActionEventDelegate SucessesfullAction;

    UPROPERTY(BlueprintAssignable)
    FWaitActionEventDelegate FailedAction;

    /**
     * Espera até que o evento de gameplay tag especificado seja acionado.
     * Por padrão, verificará o dono desta habilidade. O OptionalExternalTarget pode ser usado para monitorar outro ator.
     * Continuará ouvindo enquanto OnlyTriggerOnce = false.
     * Se OnlyMatchExact = false, acionará para tags aninhadas (filhas).
     *
     * @param OwningAbility A habilidade que está esperando.
     * @param InQueueItem O item da fila (FIQT_QueueItem) associado a esta ação. Contém SuccessTag, FailTag e TriggerTag.
     * @param InOptionalExternalTarget O ator cujo AbilitySystemComponent será monitorado, se diferente do OwningAbility.
     * @param InOnlyTriggerOnce Se verdadeiro, a task será encerrada após a primeira ocorrência de sucesso ou falha.
     * @param InOnlyMatchExact Se verdadeiro, a tag do evento deve corresponder exatamente. Se falso, tags aninhadas também acionarão.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT|Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UIQT_WaitForAction* IQT_WaitActionEvent(UGameplayAbility* OwningAbility, 
        const FIQT_QueueItem& InQueueItem,
        AActor* InOptionalExternalTarget = nullptr,
        bool InOnlyTriggerOnce = false,
        bool InOnlyMatchExact = true);

    // Helpers
    TObjectPtr<UAbilitySystemComponent> GetTargetASC() const; 

    /**
     * Retorna os dados pré-preparados para disparar o evento de gameplay associado a esta task.
     * Útil para obter a TriggerTag, Payload e Target ANTES de ativar a task para disparar um evento.
     */
    UFUNCTION(BlueprintPure, Category = "IQT|Ability|Tasks")
    const FIQT_TriggerData& GetEventTriggerData() const { return TriggerDataToUse; }

    // Overrides
    virtual void Activate() override; 
    virtual void OnDestroy(bool AbilityEnding) override;

protected:
    // O item da fila que esta tarefa está esperando
    FIQT_QueueItem QueueItemData;

    // Dados pré-preparados para o disparo do evento associado a este item.
    FIQT_TriggerData TriggerDataToUse;

    // Tags que estamos esperando (inicializadas do QueueItemData)
    FGameplayTag SuccessTag;
    FGameplayTag FailTag;

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> OptionalExternalTargetASC; 

    bool bUseExternalTarget;
    bool bOnlyTriggerOnce;
    bool bOnlyMatchExact;

    // Handles para as ligações de delegate
    FDelegateHandle SuccessHandle;
    FDelegateHandle FailHandle;

    // Callbacks para os eventos - Agora com a tag que combinou e o ator alvo
    void OnExactSuccessEvent(const FGameplayEventData* Payload);
    void OnExactFailEvent(const FGameplayEventData* Payload);

    void OnContainerSuccessEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload);
    void OnContainerFailEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload);
};
