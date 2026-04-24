// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDepthGraphWidget.generated.h"

class FPaintArgs;
class FSlateWindowElementList;
struct FGeometry;
class FSlateRect;
class FWidgetStyle;

struct FDepthGraphExtremaSample
{
	int32 SampleIndex = INDEX_NONE;
	float Depth = 0.0f;
	bool bIsPeak = false;
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	FLinearColor MarkerColor = FLinearColor::White;
};

UCLASS()
class IOX_API UDepthGraphWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Depth Graph")
	void SetGraphData(const TArray<float>& InDepthHistory, const TArray<FDateTime>& InTimeHistory, const TArray<float>& TotalVolumes, const float InCurrentDepth, const bool bInHasDepth);

	const TArray<FDepthGraphExtremaSample>& GetExtremaHistory() const
	{
		return ExtremaHistory;
	}

protected:
	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	TArray<float> VolumeLabels;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled
	) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Style")
	FMargin GraphPadding = FMargin(12.0f, 12.0f, 12.0f, 24.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Style")
	FLinearColor BackgroundColor = FLinearColor(0.03f, 0.03f, 0.03f, 0.70f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Style")
	FLinearColor BorderColor = FLinearColor(0.20f, 0.20f, 0.20f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Style")
	FLinearColor LineColor = FLinearColor(0.10f, 0.80f, 0.30f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Style")
	FLinearColor CurrentPointColor = FLinearColor(0.95f, 0.95f, 0.95f, 1.00f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Style", meta=(ClampMin="0.5", ClampMax="6.0"))
	float LineThickness = 2.0f;

	UPROPERTY(EditAnywhere, Category="Depth Graph|Style", meta=(ClampMin="1.0", ClampMax="20.0"))
	float CurrentPointSize = 5.0f;

	UPROPERTY(EditAnywhere, Category="Depth Graph|Style", meta=(ClampMin="16", ClampMax="2000"))
	int32 MaxHistorySamples = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis")
	float MinYAxisClamp = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis")
	float MaxYAxisClamp = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis")
	float FixedYRangePadding = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis")
	int32 SamplesForMinMaxCalculation = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis")
	float VolumeFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|YAxis")
	bool bShowVolumeOnExtrema = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Smoothing")
	bool bEnableDepthSmoothing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Smoothing", meta=(ClampMin="0.01", ClampMax="1.0", UIMin="0.05", UIMax="0.8"))
	float DepthSmoothingAlpha = 0.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Depth Graph|Extrema", meta=(ClampMin="1", ClampMax="50"))
	int32 MaxExtremaHistory = 10;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	TArray<float> DepthHistory;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	TArray<FDateTime> TimeHistory;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	float CurrentDepth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Depth Graph|Data")
	bool bHasDepth = false;

	mutable TArray<FDepthGraphExtremaSample> ExtremaHistory;
};
