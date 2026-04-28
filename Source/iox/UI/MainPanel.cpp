// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainPanel.h"
#include "Graph/UDepthGraphWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"


void UMainPanel::UpdateThoraxDepthGraph(const TArray<float>& History, const TArray<FDateTime>& TimeHistory, const TArray<float>& TotalVolumes, float LatestDepth, bool bHasDepth)
{
    WBPDepthGraph->SetGraphData(History, TimeHistory, TotalVolumes, LatestDepth, bHasDepth);
}

void UMainPanel::UpdateTotalVolume(float InTotalVolume)
{
    if (Volume)
    {
        // Display as Liters (input is mm^3 so divide by 1M)
        Volume->SetText(FText::AsNumber(InTotalVolume / 1000000.0f));
    }
}

void UMainPanel::NativeConstruct()
{
    Super::NativeConstruct();

    if (StartRec)
    {
        StartRec->OnClicked.AddDynamic(this, &UMainPanel::OnStartRecClickedInternal);
    }
}

void UMainPanel::OnStartRecClickedInternal()
{
    OnStartRecRequested.Broadcast();
}

