// IQT/Source/IQT/Public/IQT_DataTypes.h
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h" 
#include "IQT_DataTypes.generated.h" 

// Enum para o modo de enfileiramento (para customização da fila)
UENUM(BlueprintType)
enum class EIQT_QueueMode : uint8
{
    PriorityOrder   UMETA(DisplayName = "Order By Priority"), // Ordenação baseada na propriedade Priority
    FIFO            UMETA(DisplayName = "First In, First Out"), // Primeiro a entrar, primeiro a sair
    FILO            UMETA(DisplayName = "First In, Last Out")   // Primeiro a entrar, último a sair
};

/**
 * Estrutura de dados para um item na fila da IQT.
 * Usado para encapsular os dados do agente ou da tarefa de AI.
 */
USTRUCT(BlueprintType)
struct FIQT_QueueItem
{
    GENERATED_BODY()

    // Nome descritivo ou identificador do item.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    FName Name;

    // Tag de Gameplay para categorização ou identificação adicional, usada para TRIGERAR uma habilidade.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    FGameplayTag AbilityTriggerTag;

    // Tag de Gameplay que será esperada para o FIM BEM-SUCEDIDO de uma habilidade/ação associada ao item.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    FGameplayTag AbilityEndTag;

    // Tag de Gameplay que será esperada para o FIM COM FALHA de uma habilidade/ação associada ao item.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    FGameplayTag AbilityFailTag; // <-- NOVA TAG

    // Flag para indicar o estado do item (aberto/fechado), conforme sua lógica de AI.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    bool bIsOpen; 

    // Prioridade do item. Usado para filas de prioridade.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    int32 Priority;

    // ID de tarefa único, útil para identificação e busca.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    FGuid TaskID;

    // Flags de estado interno do item, conforme sua lógica.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    bool bIsEnqueued;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    bool bIsStacked;

    // Payload genérico para o usuário armazenar qualquer UObject que desejar associar a este item da fila.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT|Queue Item")
    UObject* UserPayload;

    // Construtor padrão
    FIQT_QueueItem()
        : Name(NAME_None)
        , AbilityTriggerTag(FGameplayTag())
        , AbilityEndTag(FGameplayTag())
        , AbilityFailTag(FGameplayTag()) // <-- INICIALIZAÇÃO DA NOVA TAG
        , bIsOpen(false)
        , Priority(0)
        , TaskID(FGuid::NewGuid()) 
        , bIsEnqueued(false)
        , bIsStacked(false)
        , UserPayload(nullptr)
    {}

    // Sobrecarga do operador de igualdade para comparação de itens
    // NOTA: 'AbilityEndTag' e 'AbilityFailTag' não estão incluídas na comparação de igualdade,
    // pois geralmente são tags de "resultado" ou "estado final" e não definem a unicidade fundamental do item na fila.
    // A unicidade é baseada no Nome, AbilityTriggerTag e estado bIsOpen.
    bool operator==(const FIQT_QueueItem& Other) const
    {
        return Name == Other.Name && AbilityTriggerTag.MatchesTagExact(Other.AbilityTriggerTag) && bIsOpen == Other.bIsOpen;
    }

    // Sobrecarga dos operadores de comparação para ordenar a fila por prioridade
    bool operator<(const FIQT_QueueItem& Other) const
    {
        return Priority < Other.Priority;
    }

    bool operator>(const FIQT_QueueItem& Other) const
    {
        return Priority > Other.Priority;
    }
};
