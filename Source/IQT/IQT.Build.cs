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

        // Includes p�blicos
        PublicIncludePaths.AddRange(new string[] {
            Path.Combine(EngineDirectory, "Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public"),
            Path.Combine(EngineDirectory, "Source/Runtime/CoreUObject/Public/StructUtils"),
        });

        PublicIncludePaths.AddRange(new string[] {
            "$(ModuleDir)/Public",
            "$(EngineDir)/Source/Runtime/Core/Public",
            "$(EngineDir)/Source/Runtime/Core/Public/Misc"
        });

        PrivateIncludePaths.AddRange(
            new string[] {
                // Adicione outros caminhos de include privados aqui, se necessário
                "$(ModuleDirectory)/Private/Internal" // Permite incluir os headers da lógica interna
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
