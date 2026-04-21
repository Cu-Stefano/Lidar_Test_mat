// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Graph/UDepthGraphWidget.h"
#include "Graph/GraphTypes.h"
#include "MainPanel.generated.h"

class UDepthGraphWidget;
class UTextBlock;

UCLASS()
class IOX_API UMainPanel : public UUserWidget
{
	GENERATED_BODY()

public:

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UDepthGraphWidget> WBPDepthGraph;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Volume;

	UFUNCTION(BlueprintCallable, Category="UI|MainPanel")
	void UpdateThoraxDepthGraph(const TArray<float>& History, const TArray<FGraphLabel>& Labels, float LatestDepth, bool bHasDepth);

    UFUNCTION(BlueprintCallable, Category="UI|MainPanel")
    void UpdateTotalVolume(float TotalVolume);


};
