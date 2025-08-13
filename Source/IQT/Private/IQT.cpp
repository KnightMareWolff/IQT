// IQT/Source/IQT/Private/IQT.cpp
// -------------------------------------------------------------------------------
// Copyright 2025 William Wolff. All Rights Reserved.
// This code is property of William Wolff and protected by copyright law.
// -------------------------------------------------------------------------------

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FIQTModule: Main module class for the IQT plugin.
 * This class implements the IModuleInterface and is responsible for
 * handling module startup and shutdown.
 */
class FIQTModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override
    {
        // This code will execute after your module is loaded into memory;
        // the exact timing is specified in the .uplugin file per-module
        UE_LOG(LogTemp, Log, TEXT("IQT Module: StartupModule called. IQT Plugin is initializing."));
    }

    virtual void ShutdownModule() override
    {
        // This function may be called during shutdown to clean up your module.
        // For modules that are dynamically unloaded, we must clean up anything that was loaded.
        UE_LOG(LogTemp, Log, TEXT("IQT Module: ShutdownModule called. IQT Plugin is shutting down."));
    }
};

// This macro implements the module.
// The first parameter is the name of your module (as defined in IQT.Build.cs).
// The second parameter is the class that implements IModuleInterface.
IMPLEMENT_MODULE(FIQTModule, IQT);
