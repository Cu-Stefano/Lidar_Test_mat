// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPanel.h"
#include "UDepthGraphWidget.h"


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

