// IQT/Source/IQT/Public/IQT_Queue.h
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h" 
#include "IQT_DataTypes.h" 

#include "IQT_Queue.generated.h" 

class UIQT_PriorityQueueInternal; 

DECLARE_LOG_CATEGORY_EXTERN(LogIOTQueue, Log, All);
/**
 * UIQT_Queue: Componente Gerenciador de Fila de Prioridade para Unreal Engine.
 * Este componente encapsula a lógica de fila C++ e a expõe para Blueprints.
 * Oferece funcionalidades de enfileiramento, desenfileiramento, e configurações
 * de comportamento (FIFO, FILO, prioridade, ignorar duplicados).
 *
 * Para usar: Adicione este componente a qualquer Actor em seu Blueprint ou C++.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class IQT_API UIQT_Queue : public UActorComponent 
{
    GENERATED_BODY()

public:
    UIQT_Queue();
    // Destrutor explicitamente declarado e definido fora da linha para resolver TUniquePtr com tipo completo
    virtual ~UIQT_Queue(); 
    virtual void BeginDestroy() override; // Limpeza do objeto interno da fila

    // --- Propriedades Configuráveis da Fila (Expostas no Blueprint) ---

    // Define o modo de enfileiramento (Prioridade, FIFO, FILO).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Queue Configuration")
    EIQT_QueueMode EnqueueMode; 

    // Se verdadeiro, elementos com Nome, Tag e bIsOpen idênticos não serão adicionados novamente.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Queue Configuration",
              meta = (ToolTip = "If true, elements with matching Name, Tag, and IsOpen status will not be added if already present."))
    bool bIgnoreDuplicatesOnEnqueue;

    // Tamanho máximo que a fila pode atingir. Se for 0, não há limite de tamanho.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Queue Configuration",
              meta = (ClampMin = "0", ToolTip = "Maximum number of items the queue can hold. 0 means no limit."))
    int32 MaxQueueSize;
    
    // --- Funções Expostas para Blueprint ---

    /**
     * Inicializa a fila. Deve ser chamada antes de qualquer operação de enfileiramento/desenfileiramento.
     * Limpa qualquer estado anterior da fila.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT Queue")
    void InitializeQueue();

    /**
     * Adiciona um item à fila. O comportamento (prioridade, FIFO, FILO) depende do 'EnqueueMode'.
     * @param ItemToEnqueue O item (FIQT_QueueItem) a ser adicionado.
     * @return True se o item foi adicionado com sucesso, false caso contrário (ex: fila cheia, duplicado).
     */
    UFUNCTION(BlueprintCallable, Category = "IQT Queue", meta=(DisplayName="Enqueue Item", Keywords="add queue push"))
    bool EnqueueItem(UPARAM(ref) FIQT_QueueItem& ItemToEnqueue); 

    /**
     * Remove e retorna o item de maior prioridade (ou o próximo em ordem FIFO/FILO) da fila.
     * @param OutItem O item removido da fila. Será inválido se a fila estiver vazia.
     * @return True se um item foi removido com sucesso, false se a fila estiver vazia.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT Queue", meta=(DisplayName="Dequeue Item", Keywords="remove queue pop"))
    bool DequeueItem(FIQT_QueueItem& OutItem); 

    /**
     * Remove um item específico da fila.
     * @param ItemToRemove O item a ser removido (comparado por Nome, Tag, bIsOpen).
     * @return True se o item foi encontrado e removido, false caso contrário.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT Queue", meta=(DisplayName="Remove Specific Item", Keywords="remove queue delete"))
    bool RemoveSpecificItem(UPARAM(ref) FIQT_QueueItem& ItemToRemove); 

    /**
     * Verifica se a fila contém um item específico (comparado por Nome, Tag, bIsOpen).
     * @param ItemToCheck O item a ser verificado.
     * @return True se o item estiver na fila, false caso contrário.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue", meta=(DisplayName="Contains Item", Keywords="queue check exists"))
    bool ContainsItem(UPARAM(ref) FIQT_QueueItem& ItemToCheck) const; 

    /**
     * Retorna o número atual de itens na fila.
     * @return A contagem de itens na fila.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue", meta=(DisplayName="Get Queue Count", Keywords="queue size count length"))
    int32 GetQueueCount() const;

    /**
     * Verifica se a fila está vazia.
     * @return True se a fila não contém itens, false caso contrário.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue", meta=(DisplayName="Is Queue Empty?", Keywords="queue empty check"))
    bool IsQueueEmpty() const;

    /**
     * Esvazia completamente a fila, removendo todos os itens.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT Queue", meta=(DisplayName="Empty Queue", Keywords="clear queue reset"))
    void EmptyQueue();

    /**
     * Retorna o número de itens na fila marcados como "abertos" (bIsOpen = true).
     * @return A contagem de itens abertos.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue|Stats", meta=(DisplayName="Get Number of Open Items", Keywords="queue open count"))
    int32 GetNumOpenItems() const;

    /**
     * Retorna o número de itens na fila marcados como "fechados" (bIsOpen = false).
     * @return A contagem de itens fechados.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue|Stats", meta=(DisplayName="Get Number of Closed Items", Keywords="queue closed count"))
    int32 GetNumClosedItems() const;

    /**
     * Valida os dados de um FIQT_QueueItem para garantir que estejam em um formato aceitável para a fila.
     * @param ItemToValidate O item a ser validado.
     * @return True se o item é considerado válido, false caso contrário.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue", meta=(DisplayName="Validate Queue Item Data", ToolTip="Checks if the provided item data is valid for enqueueing (e.g., Name is not None)."))
    bool ValidateQueueItemData(UPARAM(ref) const FIQT_QueueItem& ItemToValidate) const; 

    /**
     * Busca um item na fila pelo seu TaskID.
     * @param TaskID O GUID da tarefa a ser buscada.
     * @param OutItem O item encontrado, se houver. Será um item padrão se não encontrado.
     * @return True se um item com o TaskID especificado foi encontrado, false caso contrário.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue|Search", meta=(DisplayName="Find Item by Task ID", Keywords="queue search find TaskID"))
    bool FindItemByTaskID(const FGuid& TaskID, FIQT_QueueItem& OutItem) const; 

    /**
     * Busca um item na fila por sua "chave hash" (Nome, Tag, bIsOpen).
     * @param InName Nome do item.
     * @param InTag Tag de Gameplay do item.
     * @param bInIsOpen Estado de abertura do item.
     * @param OutItem O item encontrado, se houver. Será um item padrão se não encontrado.
     * @return True se um item com a chave especificada foi encontrado, false caso contrário.
     */
    UFUNCTION(BlueprintPure, Category = "IQT Queue|Search", meta=(DisplayName="Find Item by Hash Key", Keywords="queue search find hash key"))
    bool FindItemByHashKey(FName InName, FGameplayTag InTag, bool bInIsOpen, FIQT_QueueItem& OutItem) const; 


private:
    TUniquePtr<UIQT_PriorityQueueInternal> InternalQueue; 

    mutable int32 NextFIFOPriorityCounter;
    mutable int32 NextFILOPriorityCounter; 
};
