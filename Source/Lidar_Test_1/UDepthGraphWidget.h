// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <CoreFoundation/CFBase.h>
#include "UDepthGraphWidget.generated.h"

/**
 * 
 */
UCLASS()
class LIDAR_TEST_1_API UUDepthGraphWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Depth Graph")
	void SetGraphData(const TArray<int32>& InDepthHistoryMillimeters, int32 InCurrentDepthMillimeters, bool bInHasDepth);

protected:
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled
	) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style")
	FMargin GraphPadding = FMargin(12.0f, 12.0f, 12.0f, 12.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style")
	FLinearColor BackgroundColor = FLinearColor(0.03f, 0.03f, 0.03f, 0.70f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style")
	FLinearColor BorderColor = FLinearColor(0.20f, 0.20f, 0.20f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style")
	FLinearColor LineColor = FLinearColor(0.10f, 0.80f, 0.30f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style")
	FLinearColor CurrentPointColor = FLinearColor(0.95f, 0.95f, 0.95f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style")
	FLinearColor RecentRedExtremsColor = FLinearColor(1.00f, 0.20f, 0.20f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style", meta=(ClampMin="0.5", ClampMax="6.0"))
	float LineThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style", meta=(ClampMin="1.0", ClampMax="20.0"))
	float CurrentPointSize = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style", meta=(ClampMin="2.0", ClampMax="24.0"))
	float RecentRedExtremsMarkerSize = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Depth Graph|Style", meta=(ClampMin="16", ClampMax="2000"))
	int32 MaxHistorySamples = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis", meta=(ClampMin="0", ClampMax="5000", UIMin="0", UIMax="200"))
	int32 FixedYRangePaddingMillimeters = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|RedExtrems", meta=(ClampMin="2", ClampMax="20000", UIMin="50", UIMax="5000"))
	int32 RecentRedExtremsSampleWindow = 300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|RedExtrems", meta=(ClampMin="1", ClampMax="5000", UIMin="1", UIMax="120"))
	int32 RecentRedExtremsAddEverySamples = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|RedExtrems", meta=(ClampMin="2", ClampMax="5000", UIMin="20", UIMax="800"))
	int32 MaxPersistentRedExtremsMarkers = 400;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Smoothing")
	bool bEnableDepthSmoothing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Smoothing", meta=(ClampMin="0.01", ClampMax="1.0", UIMin="0.05", UIMax="0.8"))
	float DepthSmoothingAlpha = 0.25f;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	TArray<int32> DepthHistoryMillimeters;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	int32 CurrentDepthMillimeters = 0;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	bool bHasDepth = false;

private:
	TArray<int32> RecentDepthMillimeters;

	mutable TArray<FIntPoint> PersistentRedExtremsMarkers;
	mutable int32 LastPersistentMarkerTotalSamples = 0;
	mutable int32 SamplesSinceLastPersistentMarker = 0;
};
