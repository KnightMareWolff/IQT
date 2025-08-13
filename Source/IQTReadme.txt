# IQT: Insane Queue & Task

![IQT Logo - Insane Queue & Task](https://insaneframework.com/wp-content/uploads/2025/08/LogoIQT-300x140.png)

IQT (Insane Queue & Task) is a robust, high-performance, and incredibly reliable queuing and task management system designed to handle asynchronous operations and background processes with unmatched efficiency. Part of the "Insane" suite of tools (alongside IVR and IAR), IQT empowers developers to build highly scalable and responsive applications by offloading heavy computational tasks, ensuring seamless execution even under extreme load.

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

Here's a conceptual example demonstrating how you might define and enqueue a task within an Unreal Engine application:

```cpp
// 1. Define your task data (payload) using a USTRUCT
//    This allows easy passing of parameters for your task.
USTRUCT(BlueprintType)
struct FIQTImageProcessTaskData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Tasks")
    FString ImagePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Tasks")
    FString FilterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IQT Tasks")
    int32 QualitySetting;
};

// 2. Define a conceptual IQT management class (e.g., a UGameInstanceSubsystem)
//    This provides global access to your IQT system for enqueuing tasks.
UCLASS()
class UMyIQTSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Conceptual method to enqueue a task
    // TaskFunction: A TFunction<void()> representing the actual work to be done.
    // TaskData: The specific data payload for this task.
    void EnqueueIQTTask(TFunction<void()> TaskFunction, const FIQTImageProcessTaskData& TaskData)
    {
        // In a real IQT implementation:
        // - TaskData would be serialized and sent to a message queue (e.g., RabbitMQ, Kafka).
        // - TaskFunction (or a reference to it) would be handled by a worker pool.
        // - Workers (separate threads/processes) would dequeue tasks and execute TaskFunction.

        UE_LOG(LogTemp, Log, TEXT("IQT: Enqueuing task for image '%s' with filter '%s'"),
               *TaskData.ImagePath, *TaskData.FilterType);

        // For demonstration, we'll simulate immediate execution or a simple async call within Unreal.
        // In a real IQT, this would trigger robust background processing across your system.
        Async(EAsyncExecution::Thread, TaskFunction);
    }
};

// 3. Enqueuing a task from an Unreal Actor or Blueprint Callable function
//    This shows how you'd trigger a background operation from your game logic.
UCLASS()
class AMyGameActor : public AActor
{
    GENERATED_BODY()

public:
    AMyGameActor() {}

    UFUNCTION(BlueprintCallable, Category = "My Game")
    void TriggerImageProcessingTask(const FString& ImagePath, const FString& Filter)
    {
        // Get the IQT Subsystem from the current Game Instance
        UMyIQTSubsystem* IQTSubsystem = GetGameInstance()->GetSubsystem<UMyIQTSubsystem>();
        if (IQTSubsystem)
        {
            // Prepare the data for our image processing task
            FIQTImageProcessTaskData TaskData;
            TaskData.ImagePath = ImagePath;
            TaskData.FilterType = Filter;
            TaskData.QualitySetting = 90;

            // Define the actual heavy work as a lambda function.
            // This lambda will be executed on a background thread by IQT.
            auto ImageProcessingLogic = [TaskData]()
            {
                // --- THIS CODE RUNS ON A BACKGROUND THREAD MANAGED BY IQT ---
                UE_LOG(LogTemp, Log, TEXT("IQT Worker: Starting image processing for '%s' with filter '%s'"),
                       *TaskData.ImagePath, *TaskData.FilterType);

                // Simulate a heavy, long-running operation (e.g., resizing, applying complex filters, disk I/O)
                FPlatformProcess::Sleep(3.0f); // Simulate 3 seconds of work

                UE_LOG(LogTemp, Log, TEXT("IQT Worker: Finished image processing for '%s'"),
                       *TaskData.ImagePath);

                // If results need to be visible or used on the game thread (e.g., update UI, load new texture):
                // You would typically dispatch a callback or event back to the game thread.
                // FSimpleDelegate::CreateLambda([TaskData]() {
                //     // This lambda runs on the game thread (main thread)
                //     UE_LOG(LogTemp, Log, TEXT("IQT Game Thread Callback: Image '%s' processing complete, updating UI."), *TaskData.ImagePath);
                //     // Example: UTexture2D* NewTexture = LoadTextureFromDisk(TaskData.ImagePath);
                //     // Example: UMyGameWidget->UpdateImage(NewTexture);
                // }).ExecuteIfBound();
            };

            // Enqueue the task with IQT
            IQTSubsystem->EnqueueIQTTask(ImageProcessingLogic, TaskData);
            UE_LOG(LogTemp, Warning, TEXT("Image processing task enqueued successfully!"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("IQT Subsystem not found! Make sure it's initialized."));
        }
    }
};
