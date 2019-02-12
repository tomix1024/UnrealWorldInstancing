// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WorldInstancingGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class WORLDINSTANCING_API AWorldInstancingGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	virtual void Tick(float DeltaSeconds) override;

	void CreateNewGameWindow(int WindowX, int WindowY, int WindowWidth, int WindowHeight, FString WindowTitle);

private:

	TArray<TSharedPtr<SWindow>> SlateWindows;
	TArray<TSharedPtr<SViewport>> NewViewportWidgets;
	TArray<TSharedPtr<FSceneViewport>> NewSceneViewports;
};
