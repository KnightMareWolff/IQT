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
#include "AbilitySystemComponent.h" // Necess�rio para FGameplayEventTagDelegate (para o delegate de broadcast)
#include "IQT_WaitForAction.generated.h" 

class UAbilitySystemComponent;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitActionEventDelegate, FGameplayEventData, Payload);

/**
 * UIQT_WaitForAction: Uma AbilityTask que espera por eventos de Gameplay Tag.
 * Permite que uma Gameplay Ability pause sua execu��o at� que uma tag de sucesso ou falha seja emitida.
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
     * Espera at� que o evento de gameplay tag especificado seja acionado.
     * Por padr�o, verificar� o dono desta habilidade. O OptionalExternalTarget pode ser usado para monitorar outro ator.
     * Continuar� ouvindo enquanto OnlyTriggerOnce = false.
     * Se OnlyMatchExact = false, acionar� para tags aninhadas (filhas).
     *
     * @param OwningAbility A habilidade que est� esperando.
     * @param SuccessTag A tag que, se emitida, acionar� o sucesso.
     * @param FailTag A tag que, se emitida, acionar� a falha.
     * @param OptionalExternalTarget O ator cujo AbilitySystemComponent ser� monitorado, se diferente do OwningAbility.
     * @param OnlyTriggerOnce Se verdadeiro, a task ser� encerrada ap�s a primeira ocorr�ncia de sucesso ou falha.
     * @param OnlyMatchExact Se verdadeiro, a tag do evento deve corresponder exatamente. Se falso, tags aninhadas tamb�m acionar�o.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT|Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UIQT_WaitForAction* IQT_WaitActionEvent(UGameplayAbility* OwningAbility, 
        FGameplayTag SuccessTag,
        FGameplayTag FailTag,
        AActor* OptionalExternalTarget = nullptr,
        bool OnlyTriggerOnce = false,
        bool OnlyMatchExact = true);

    // Helpers
    UAbilitySystemComponent* GetTargetASC() const;

    // Overrides
    virtual void Activate() override;
    virtual void OnDestroy(bool AbilityEnding) override;

protected:
    // Tags que estamos esperando
    FGameplayTag SuccessTag;
    FGameplayTag FailTag;

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> OptionalExternalTargetASC; 

    bool bUseExternalTarget;
    bool bOnlyTriggerOnce;
    bool bOnlyMatchExact;

    // Handles para as liga��es de delegate
    FDelegateHandle SuccessHandle;
    FDelegateHandle FailHandle;

    // Callbacks para os eventos - Assinaturas AGORA CORRETAS
    // Para bOnlyMatchExact == true (GenericGameplayEventCallbacks)
    void OnExactSuccessEvent(const FGameplayEventData* Payload);
    void OnExactFailEvent(const FGameplayEventData* Payload);

    // Para bOnlyMatchExact == false (AddGameplayEventTagContainerDelegate)
    void OnContainerSuccessEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload);
    void OnContainerFailEvent(FGameplayTag MatchedTag, const FGameplayEventData* Payload);
};
