// IQT/Source/IQT/IQT.Build.cs
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

using System.IO;
using UnrealBuildTool;

public class IQT : ModuleRules
{
    public IQT(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Includes públicos
        // Use Path.Combine com ModuleDirectory para caminhos relativos ao seu módulo
        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(EngineDirectory, "Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public"),
            Path.Combine(EngineDirectory, "Source/Runtime/CoreUObject/Public/StructUtils"),
            Path.Combine(ModuleDirectory, "Public"), // <--- Mudei este
            Path.Combine(EngineDirectory, "Source/Runtime/Core/Public"),
            Path.Combine(EngineDirectory, "Source/Runtime/Core/Public/Misc")
        });

        PrivateIncludePaths.AddRange(
            new string[] {
                // Adicione outros caminhos de include privados aqui, se necessário
                Path.Combine(ModuleDirectory, "Private", "Internal") // <--- ESTA É A CORREÇÃO PRINCIPAL: USE Path.Combine(ModuleDirectory, ...)
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // Adicione módulos que são carregados dinamicamente aqui.
            }
            );
    }
}