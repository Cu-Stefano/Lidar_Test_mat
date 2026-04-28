// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainPanel.generated.h"

class UDepthGraphWidget;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartRecRequested);

UCLASS()
class IOX_API UMainPanel : public UUserWidget
{
	GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnStartRecRequested OnStartRecRequested;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UDepthGraphWidget> WBPDepthGraph;
    
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UTextBlock> Volume;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    TObjectPtr<UButton> StartRec;

	UFUNCTION(BlueprintCallable, Category="UI|MainPanel")
	void UpdateThoraxDepthGraph(const TArray<float>& History, const TArray<FDateTime>& TimeHistory, const TArray<float>& TotalVolumes, float LatestDepth, bool bHasDepth);

    UFUNCTION(BlueprintCallable, Category="UI|MainPanel")
    void UpdateTotalVolume(float TotalVolume);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnStartRecClickedInternal();
};
