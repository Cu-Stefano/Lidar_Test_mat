// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainPanel.h"
#include "Graph/UDepthGraphWidget.h"
#include "Components/TextBlock.h"


void UMainPanel::UpdateThoraxDepthGraph(const TArray<float>& History, float LatestDepth, bool bHasDepth)
{
    WBPDepthGraph->SetGraphData(History, LatestDepth, bHasDepth);
    WBPDepthGraph_1->SetGraphData(History, LatestDepth, bHasDepth);
}

void UMainPanel::UpdateSternumDepthGraph(const TArray<float>& History, float LatestDepth, bool bHasDepth)
{
    WBPSternumGraph->SetGraphData(History, LatestDepth, bHasDepth);
    WBPSternumGraph_1->SetGraphData(History, LatestDepth, bHasDepth);
}

void UMainPanel::UpdateTotalVolume(float InTotalVolume)
{
    if (Volume)
    {
        // Display as Liters (input is mm^3 so divide by 1M)
        Volume->SetText(FText::AsNumber(InTotalVolume / 1000000.0f));
    }
}

