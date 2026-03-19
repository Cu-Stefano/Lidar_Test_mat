// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainPanel.generated.h"

class UUDepthGraphWidget;

/**
 * 
 */
UCLASS()
class IOX_API UMainPanel : public UUserWidget
{
	GENERATED_BODY()

public:
    // Variabili che corrispondono ai widget nel designer
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUDepthGraphWidget> WBPDepthGraph;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUDepthGraphWidget> WBPSternumGraph;

	UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUDepthGraphWidget> WBPDepthGraph_1;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUDepthGraphWidget> WBPSternumGraph_1;

	UFUNCTION(BlueprintCallable, Category="UI|MainPanel")
	void UpdateThoraxDepthGraph(const TArray<float>& History, float LatestDepth, bool bHasDepth);

	UFUNCTION(BlueprintCallable, Category="UI|MainPanel")
	void UpdateSternumDepthGraph(const TArray<float>& History, float LatestDepth, bool bHasDepth);


};
