// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldInstancingGameModeBase.h"

#include "WorldInstancing.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"

#include "Widgets/SWindow.h"
#include "Widgets/SViewport.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Slate/SceneViewport.h"

#include "Misc/FileHelper.h"
#include "ImageUtils.h"

void AWorldInstancingGameModeBase::BeginPlay()
{
    UE_LOG(LogWorldInstancing, Log, TEXT("Begin Play"));

    // Create two new game instances
    CreateNewGameWindow(0, 0, 640, 480, TEXT("Second Window"));
    CreateNewGameWindow(640, 0, 640, 480, TEXT("Third Window"));

    Super::BeginPlay();
}

void AWorldInstancingGameModeBase::CreateNewGameWindow(int WindowX, int WindowY, int WindowWidth, int WindowHeight, FString WindowTitle)
{
    UE_LOG(LogWorldInstancing, Log, TEXT("CreateNewGameInstance"));

    const FURL MapURL(TEXT("/Game/SubLevel?Name=Player"));
    const FString MapName = TEXT("/Game/SubLevel");

    // Create GameInstance

    UGameInstance* NewGameInstance = NewObject<UGameInstance>(GEngine);
    NewGameInstance->AddToRoot();

    NewGameInstance->InitializeStandalone();
    FWorldContext* NewWorldContext = NewGameInstance->GetWorldContext();

    // Assign PIE Instance (required for what we want to do)!
    static int InstanceID = 0;
    NewWorldContext->PIEInstance = ++InstanceID;

    NewGameInstance->RemoveFromRoot();

    // Create GameViewportClient

    UGameViewportClient* NewGameViewportClient = NewObject<UGameViewportClient>(GEngine, GEngine->GameViewportClientClass);
    NewGameViewportClient->Init(*NewWorldContext, NewGameInstance);
    NewWorldContext->GameViewport = NewGameViewportClient;

    // Create ViewportWidget

    TSharedPtr<SViewport> NewViewportWidget = SNew(SViewport)
        .RenderDirectlyToWindow(false)
        .IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
        .EnableGammaCorrection(false)   // Scene rendering handles gamma correction
        .EnableBlending(false);

    // Create Window

    TSharedPtr<SWindow> SlateWindow = SNew(SWindow)
        .AutoCenter(EAutoCenter::None)
        .Title(FText::FromString(WindowTitle))
        .IsInitiallyMaximized(false)
        .ScreenPosition(FVector2D(WindowX, WindowY))
        .ClientSize(FVector2D(WindowWidth, WindowHeight))
        .CreateTitleBar(true)
        .SizingRule(ESizingRule::UserSized)
        .SupportsMaximize(false)
        .SupportsMinimize(true)
        .HasCloseButton(true);

    FSlateApplication& SlateApp = FSlateApplication::Get();
    SlateApp.AddWindow(SlateWindow.ToSharedRef(), true);

    FSlateRenderer *Renderer = FSlateApplication::Get().GetRenderer();
    Renderer->CreateViewport(SlateWindow.ToSharedRef());
    SlateWindow->ShowWindow();

    // Add ViewportWidget to Window

    SlateWindow->SetContent(NewViewportWidget.ToSharedRef());

    // Create SceneViewport

    TSharedPtr<FSceneViewport> NewSceneViewport = MakeShareable(new FSceneViewport(NewGameViewportClient, NewViewportWidget));
    NewGameViewportClient->Viewport = NewSceneViewport.Get();

    NewViewportWidget->SetViewportInterface(NewSceneViewport.ToSharedRef());
    FViewportFrame* NewViewportFrame = NewSceneViewport.Get();
    NewGameViewportClient->SetViewportFrame(NewViewportFrame);

    // NewSceneViewport->ResizeFrame(640, 640, GSystemResolution.WindowMode);
    // NewSceneViewport->SetFixedViewportSize(640, 480);
    // NewSceneViewport->UpdateViewportRHI(false, WindowWidth, WindowHeight, GSystemResolution.WindowMode, PF_Unknown);
    FSlateApplication::Get().RegisterGameViewport(NewViewportWidget.ToSharedRef());


    // Create local player

    FString Error;
    if(NewGameViewportClient->SetupInitialLocalPlayer(Error) == NULL)
    {
        UE_LOG(LogWorldInstancing, Fatal, TEXT("%s"), *Error);
    }

    UGameViewportClient::OnViewportCreated().Broadcast();


    // Make sure that the level is already loaded in memory

    UPackage *TemplateWorldPackage = FindPackage(nullptr, *MapName);
    if (TemplateWorldPackage == nullptr)
    {
        TemplateWorldPackage = LoadPackage(nullptr, *MapName, LOAD_None);
    }

    UWorld *TemplateWorld = UWorld::FindWorldInPackage(TemplateWorldPackage);
    if (!TemplateWorld->bIsWorldInitialized)
    {
        TemplateWorld->InitWorld();
        TemplateWorld->AddToRoot();
    }


    // StartGameInstance()
    {
        UEngine* const Engine = GEngine;

        FURL DefaultURL = MapURL;

        // Enter initial world.
        EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
        FString Error;

        const FString DefaultMap = MapName;

        BrowseRet = Engine->Browse(*NewWorldContext, FURL(&DefaultURL, *DefaultMap, TRAVEL_Partial), Error);

        // Handle failure.
        if (BrowseRet == EBrowseReturnVal::Failure)
        {
            UE_LOG(LogLoad, Error, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *DefaultMap, *Error));
        }
    }
    // End StartGameInstance()

    ULevel* NewLevel = NewWorldContext->World()->GetCurrentLevel();
    if (NewLevel == nullptr)
    {
        UE_LOG(LogWorldInstancing, Fatal, TEXT("Null Level!"));
    }
    for (int i = 0; i < NewLevel->Actors.Num(); ++i)
    {
        AActor *Actor = NewLevel->Actors[i];
        if (Actor == nullptr)
        {
            UE_LOG(LogWorldInstancing, Log, TEXT("Actor Null"));
        }
        else
        {
            UE_LOG(LogWorldInstancing, Log, TEXT("Actor %s"), *Actor->GetName());
        }
    }

    // Remember shared pointers to avoid deallocation...
    SlateWindows.Add(SlateWindow);
    NewViewportWidgets.Add(NewViewportWidget);
    NewSceneViewports.Add(NewSceneViewport);

    UE_LOG(LogWorldInstancing, Log, TEXT("End CreateNewGameInstance"));
}

void AWorldInstancingGameModeBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AWorldInstancingGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}
