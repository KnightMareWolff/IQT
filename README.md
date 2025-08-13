# IQT: Insane Queue & Task

![IQT Logo - Insane Queue & Task](https://insaneframework.com/wp-content/uploads/2025/08/LogoIQT-300x140.png)

IQT (Insane Queue & Task) is a robust, high-performance, and incredibly reliable queuing and task management system designed to handle asynchronous operations and background processes with unmatched efficiency. Part of the "Insane" suite of tools ( alongside IVR and IAR), IQT empowers developers to build highly scalable and responsive applications by offloading heavy computational tasks, ensuring seamless execution even under extreme load.

## ✨ Features

*   **Insane Performance:** Engineered for lightning-fast task processing, IQT ensures your background jobs are executed with minimal latency, keeping your applications responsive and efficient.
*   **Rock-Solid Reliability:** With advanced retry mechanisms, robust error handling, and persistent queueing options, IQT guarantees task completion, even in the face of system failures or temporary outages.
*   **Effortless Scalability:** Designed from the ground up to scale horizontally, IQT can effortlessly manage millions of tasks across distributed environments, adapting to your application's growing demands.
*   **Flexible Task Management:** Supports diverse task types, complex scheduling, and customizable prioritization, giving you granular control over your workflow.
*   **Seamless Integration:** Built with a developer-friendly API, IQT integrates smoothly into existing projects, allowing you to quickly define, enqueue, and process tasks.
*   **Decoupled Architecture:** Promotes a modular and maintainable codebase by decoupling producers from consumers, enhancing system resilience and flexibility.

## 🚀 Why Choose IQT?

In today's fast-paced digital world, applications need to be agile, responsive, and resilient. IQT provides the backbone for such systems by:

*   **Improving User Experience:** Offload long-running operations (like image processing, email sending, data analysis) to background tasks, keeping your user interface snappy and interactive.
*   **Enhancing System Resilience:** Ensure critical processes are completed reliably, even if parts of your system temporarily go offline.
*   **Optimizing Resource Usage:** Efficiently distribute workloads across your infrastructure, preventing bottlenecks and maximizing throughput.
*   **Simplifying Complex Workflows:** Break down large, monolithic tasks into smaller, manageable units that can be processed asynchronously.

## 🛠️ Getting Started (Conceptual with Unreal Engine C++)

For Unreal Engine developers, IQT would integrate seamlessly into your C++ projects, allowing you to offload heavy computations or long-running tasks to background queues, preventing hitches on your game thread.

The example below demonstrates how you would define task data and enqueue it using IQT's `UIQT_Queue` component. It also illustrates a concept of how a "worker" would consume these tasks.

**Conceptual Structure:**

1.  **`UIQTImageProcessTaskData` (Task Data):** A simple UCLASS to encapsulate the specific parameters for your task (in this case, image processing).
2.  **`AIQTManagerActor` (Queue Manager Actor):** A persistent actor in your level that would host the `UIQT_Queue` component instance.
3.  **`UMyIQTSubsystem` (Management Subsystem):** A `UGameInstanceSubsystem` that acts as a global access point to find and interact with the `UIQT_Queue` from the `AIQTManagerActor`. It orchestrates task enqueuing.
4.  **`AMyGameActor` (Task Producer):** An example actor that initiates a task, sending its data to the `UMyIQTSubsystem`.
5.  **`UMyIQTWorkerComponent` (Consumer/Worker Component):** A conceptual component that, in a loop or by event, dequeues tasks from the `UIQT_Queue` and processes them in a separate thread.

---

```cpp
// --- Conceptual: A Manager Actor that hosts the IQT Queue component ---
// This actor would typically be instantiated or placed in the world (e.g., in GameMode or as a Global Manager).
// It is essential for UIQT_Queue, being a UActorComponent, to exist in the world.
#pragma once

#include "GameFramework/Actor.h"
#include "IQT_Queue.h" // Include your UIQT_Queue component header
#include "IQTManagerActor.generated.h"

UCLASS()
class MYPROJECT_API AIQTManagerActor : public AActor
{
    GENERATED_BODY()
public:
    AIQTManagerActor()
    {
        // Creates and attaches the UIQT_Queue component
        IQTQueueComponent = CreateDefaultSubobject<UIQT_Queue>(TEXT("IQTQueueComponent"));
    }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IQT")
    TObjectPtr<UIQT_Queue> IQTQueueComponent;

    // Optional: To initialize the queue when the game starts
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        if (IQTQueueComponent)
        {
            IQTQueueComponent->InitializeQueue();
            UE_LOG(LogTemp, Log, TEXT("AIQTManagerActor: IQTQueueComponent initialized."));
        }
    }
};

// --- 1. Definition of your specific task data (Payload) ---
// We use UCLASS so it can be stored in the UObject* UserPayload of FIQT_QueueItem.
#pragma once

#include "UObject/NoExportTypes.h"
#include "IQTImageProcessTaskData.generated.h"

UCLASS(BlueprintType)
class MYPROJECT_API UIQTImageProcessTaskData : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Tasks")
    FString ImagePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Tasks")
    FString FilterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Tasks")
    int32 QualitySetting;
};


// --- 2. Definition of an IQT Management Subsystem (UGameInstanceSubsystem) ---
// This subsystem provides global and managed access to your IQT queue.
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "EngineUtils.h" // For TActorIterator
#include "IQT_Queue.h"    // Include your UIQT_Queue component header
#include "IQTImageProcessTaskData.h" // If UIQTImageProcessTaskData is a UCLASS
#include "IQT_DataTypes.h" // For FIQT_QueueItem
#include "IQTManagerActor.h" // To find the Manager Actor
#include "UMyIQTSubsystem.generated.h"

UCLASS()
class MYPROJECT_API UMyIQTSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Reference to the actual UIQT_Queue component in the world.
    UPROPERTY(Transient) // Transient: not saved to disk
    TObjectPtr<UIQT_Queue> GlobalIQTQueue;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override
    {
        Super::Initialize(Collection);
        UE_LOG(LogTemp, Log, TEXT("UMyIQTSubsystem: Initialized."));
    }

    virtual void Deinitialize() override
    {
        GlobalIQTQueue = nullptr; // Releases the reference when deallocated
        Super::Deinitialize();
    }

    // Method to get the UIQT_Queue component (searches for AIQTManagerActor in the world)
    UIQT_Queue* GetIQTQueueComponent()
    {
        if (GlobalIQTQueue.IsValid())
        {
            return GlobalIQTQueue;
        }
        
        // If the reference is not set, tries to find the AIQTManagerActor in the world.
        // Note: In a real project, you would ensure that this ManagerActor is instantiated
        // in the world (e.g., via GameMode or level placement) before being accessed.
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<AIQTManagerActor> It(World); It; ++It)
            {
                GlobalIQTQueue = It->IQTQueueComponent;
                if (GlobalIQTQueue)
                {
                    // We don't call InitializeQueue() here again as it's already called in the ManagerActor's BeginPlay
                    UE_LOG(LogTemp, Log, TEXT("UMyIQTSubsystem: UIQT_Queue component found via AIQTManagerActor."));
                    return GlobalIQTQueue;
                }
            }
        }
        UE_LOG(LogTemp, Error, TEXT("UMyIQTSubsystem: AIQTManagerActor or UIQT_Queue component not found in the world."));
        return nullptr;
    }

    /**
     * Enqueues an image processing task into the IQT system.
     * @param TaskData The specific image processing task data.
     */
    UFUNCTION(BlueprintCallable, Category = "IQT|Tasks")
    void EnqueueImageProcessingTask(UIQTImageProcessTaskData* TaskData)
    {
        if (!TaskData)
        {
            UE_LOG(LogTemp, Error, TEXT("UMyIQTSubsystem: Invalid task data for enqueuing."));
            return;
        }

        UIQT_Queue* IQTQueue = GetIQTQueueComponent();
        if (!IQTQueue)
        {
            UE_LOG(LogTemp, Error, TEXT("UMyIQTSubsystem: UIQT_Queue component not found or initialized!"));
            return;
        }

        // 1. Create an instance of FIQT_QueueItem
        FIQT_QueueItem QueueItem;
        QueueItem.Name = FName(*FString::Printf(TEXT("ImageProcess_%s"), *TaskData->ImagePath));
        QueueItem.Priority = 100; // Example priority
        // Example tag to trigger an ability or worker event.
        // Make sure this tag exists in your project (e.g., via DefaultGameplayTags.ini)
        QueueItem.AbilityTriggerTag = FGameplayTag::RequestGameplayTag(TEXT("IQT.Task.ImageProcess")); 
        QueueItem.bIsOpen = true; // New task, open for processing
        QueueItem.UserPayload = TaskData; // Stores the specific task data

        // 2. Enqueue the item using the UIQT_Queue method
        bool bSuccess = IQTQueue->EnqueueItem(QueueItem);

        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("UMyIQTSubsystem: Task for image '%s' successfully enqueued (TaskID: %s)."),
                   *TaskData->ImagePath, *QueueItem.TaskID.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UMyIQTSubsystem: Failed to enqueue task for image '%s'."), *TaskData->ImagePath);
        }
    }
};


// --- 3. Enqueue a task from an Unreal Actor or Blueprint Callable function ---
// This shows how you would trigger a background operation from your game logic.
#pragma once

#include "GameFramework/Actor.h"
#include "UMyIQTSubsystem.h" // Include the subsystem header
#include "IQTImageProcessTaskData.h" // Include the task data header
#include "AMyGameActor.generated.h"

UCLASS()
class MYPROJECT_API AMyGameActor : public AActor
{
    GENERATED_BODY()

public:
    AMyGameActor() {}

    UFUNCTION(BlueprintCallable, Category = "My Game")
    void TriggerImageProcessingTask(const FString& ImagePath, const FString& Filter)
    {
        // Get the IQT Subsystem from the current game instance
        UMyIQTSubsystem* IQTSubsystem = GetGameInstance()->GetSubsystem<UMyIQTSubsystem>();
        if (IQTSubsystem)
        {
            // Prepare the specific data for our image processing task
            UIQTImageProcessTaskData* SpecificTaskData = NewObject<UIQTImageProcessTaskData>(this); // Create a new UObject instance
            SpecificTaskData->ImagePath = ImagePath;
            SpecificTaskData->FilterType = Filter;
            SpecificTaskData->QualitySetting = 90;

            // Enqueue the task via the subsystem
            IQTSubsystem->EnqueueImageProcessingTask(SpecificTaskData);
            UE_LOG(LogTemp, Warning, TEXT("AMyGameActor: Image processing task sent to IQT Subsystem."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AMyGameActor: IQT Subsystem not found! Make sure it is initialized and accessible."));
        }
    }
};


// --- Conceptual: A Worker (e.g., another Component or Actor) that processes the queue ---
// This component simulates a worker that periodically dequeues and processes tasks.
// In a real IQT system, processing would occur in dedicated worker processes or threads,
// which could be on other machines or services.
#pragma once

#include "Components/ActorComponent.h"
#include "UMyIQTSubsystem.h"
#include "IQT_DataTypes.h"
#include "IQTImageProcessTaskData.h"
#include "MyIQTWorkerComponent.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UMyIQTWorkerComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMyIQTWorkerComponent() {}

    UPROPERTY(Transient)
    TObjectPtr<UIQT_Queue> AssociatedIQTQueue; // Link to the global IQT queue managed by the subsystem

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        // Get the IQT Subsystem and retrieve the queue it manages
        if (UMyIQTSubsystem* IQTSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMyIQTSubsystem>())
        {
            AssociatedIQTQueue = IQTSubsystem->GetIQTQueueComponent();
            if (AssociatedIQTQueue)
            {
                // Starts a timer to process the next task periodically
                GetWorld()->GetTimerManager().SetTimer(ProcessTimerHandle, this, &UMyIQTWorkerComponent::ProcessNextTask, 1.0f, true);
                UE_LOG(LogTemp, Log, TEXT("UMyIQTWorkerComponent: Processing timer started."));
            }
            else
            {
                 UE_LOG(LogTemp, Error, TEXT("UMyIQTWorkerComponent: UIQT_Queue not found for processing."));
            }
        }
    }

    void ProcessNextTask()
    {
        if (!AssociatedIQTQueue || AssociatedIQTQueue->IsQueueEmpty())
        {
            // UE_LOG(LogTemp, Verbose, TEXT("UMyIQTWorkerComponent: Queue empty or unassociated."));
            return;
        }

        FIQT_QueueItem DequeuedItem;
        if (AssociatedIQTQueue->DequeueItem(DequeuedItem))
        {
            // --- THIS CODE SIMULATES WORK ON A BACKGROUND THREAD ---
            // In a real IQT, this would be handled by dedicated worker processes/threads.
            // Here, we simulate dispatching the real work to an asynchronous task.

            // Retrieves the specific payload and casts it to the correct type
            UIQTImageProcessTaskData* Payload = Cast<UIQTImageProcessTaskData>(DequeuedItem.UserPayload);
            if (Payload)
            {
                FString ImagePath = Payload->ImagePath;
                FString FilterType = Payload->FilterType;
                FGuid TaskID = DequeuedItem.TaskID; // Captures the TaskID for worker internal logs

                // Dispatches the heavy work to a separate thread
                Async(EAsyncExecution::Thread, [ImagePath, FilterType, TaskID]()
                {
                    UE_LOG(LogTemp, Log, TEXT("IQT Worker Thread: Starting image processing for '%s' with filter '%s' (TaskID: %s)"),
                           *ImagePath, *FilterType, *TaskID.ToString());

                    FPlatformProcess::Sleep(3.0f); // Simulates 3 seconds of heavy work

                    UE_LOG(LogTemp, Log, TEXT("IQT Worker Thread: Image processing finished for '%s'."),
                           *ImagePath);

                    // If results need to be visible or used on the game thread (e.g., update UI, load new texture):
                    // You would typically dispatch a callback or event back to the game thread (Game Thread).
                    // Example: FSimpleDelegate::CreateLambda([]() { ... }).ExecuteIfBound();
                    // Would use FFunctionGraphTask::ExecuteInGameThread for Game Thread operations.
                });
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UMyIQTWorkerComponent: Dequeued item '%s' does not have a valid UIQTImageProcessTaskData as Payload."),
                       *DequeuedItem.Name.ToString());
            }
        }
    }

private:
    FTimerHandle ProcessTimerHandle;
};
