// Fill out your copyright notice in the Description page of Project Settings.


#include "UDepthGraphWidget.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UUDepthGraphWidget::SetGraphData(const TArray<int32>& InDepthHistoryMillimeters, const int32 InCurrentDepthMillimeters, const bool bInHasDepth)
{
	DepthHistoryMillimeters = InDepthHistoryMillimeters;
	CurrentDepthMillimeters = InCurrentDepthMillimeters;
	bHasDepth = bInHasDepth;

	{
		static double LastSetGraphDataLogSeconds = 0.0;
		const double NowSeconds = FPlatformTime::Seconds();
		if (NowSeconds - LastSetGraphDataLogSeconds > 0.5)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("DepthGraphWidget: SetGraphData history=%d current=%d hasDepth=%s"),
				DepthHistoryMillimeters.Num(),
				CurrentDepthMillimeters,
				bHasDepth ? TEXT("true") : TEXT("false")
			);
			LastSetGraphDataLogSeconds = NowSeconds;
		}
	}

	InvalidateLayoutAndVolatility();
}

int32 UUDepthGraphWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	const bool bParentEnabled
) const
{
	const int32 BaseLayer = Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId,
		InWidgetStyle,
		bParentEnabled
	);

	const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();
	{
		static double LastNativePaintEntryLogSeconds = 0.0;
		const double NowSeconds = FPlatformTime::Seconds();
		if (NowSeconds - LastNativePaintEntryLogSeconds > 1.0)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("DepthGraphWidget: NativePaint enter size=(%.1f, %.1f) hasDepth=%s samples=%d"),
				WidgetSize.X,
				WidgetSize.Y,
				bHasDepth ? TEXT("true") : TEXT("false"),
				DepthHistoryMillimeters.Num()
			);
			LastNativePaintEntryLogSeconds = NowSeconds;
		}
	}

	if (WidgetSize.X <= 1.0f || WidgetSize.Y <= 1.0f)
	{
		static double LastTinySizeLogSeconds = 0.0;
		const double NowSeconds = FPlatformTime::Seconds();
		if (NowSeconds - LastTinySizeLogSeconds > 1.0)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("DepthGraphWidget: NativePaint skip tiny size=(%.2f, %.2f)"),
				WidgetSize.X,
				WidgetSize.Y
			);
			LastTinySizeLogSeconds = NowSeconds;
		}
		return BaseLayer;
	}

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	FPaintGeometry FullPaintGeometry = AllottedGeometry.ToPaintGeometry();

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		BaseLayer + 1,
		FullPaintGeometry,
		WhiteBrush,
		ESlateDrawEffect::None,
		BackgroundColor
	);

	const FVector2D BorderSize(WidgetSize.X - 2.0f, WidgetSize.Y - 2.0f);
	if (BorderSize.X > 0.0f && BorderSize.Y > 0.0f)
	{
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			BaseLayer + 2,
			AllottedGeometry.ToPaintGeometry(),
			{
				FVector2D(1.0f, 1.0f),
				FVector2D(1.0f + BorderSize.X, 1.0f),
				FVector2D(1.0f + BorderSize.X, 1.0f + BorderSize.Y),
				FVector2D(1.0f, 1.0f + BorderSize.Y),
				FVector2D(1.0f, 1.0f)
			},
			ESlateDrawEffect::None,
			BorderColor,
			true,
			1.0f
		);
	}

	if (!bHasDepth || DepthHistoryMillimeters.Num() < 2)
	{
		static double LastNoDataPaintLogSeconds = 0.0;
		const double NowSeconds = FPlatformTime::Seconds();
		if (NowSeconds - LastNoDataPaintLogSeconds > 1.0)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("DepthGraphWidget: NativePaint skip (hasDepth=%s, samples=%d)"),
				bHasDepth ? TEXT("true") : TEXT("false"),
				DepthHistoryMillimeters.Num()
			);
			LastNoDataPaintLogSeconds = NowSeconds;
		}
		return BaseLayer + 2;
	}

	const float Left = GraphPadding.Left;
	const float Top = GraphPadding.Top;
	const float Right = WidgetSize.X - GraphPadding.Right;
	const float Bottom = WidgetSize.Y - GraphPadding.Bottom;
	const float PlotWidth = Right - Left;
	const float PlotHeight = Bottom - Top;

	if (PlotWidth <= 2.0f || PlotHeight <= 2.0f)
	{
		static double LastSmallPlotLogSeconds = 0.0;
		const double NowSeconds = FPlatformTime::Seconds();
		if (NowSeconds - LastSmallPlotLogSeconds > 1.0)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("DepthGraphWidget: NativePaint skip small plot=(%.2f, %.2f) size=(%.2f, %.2f) padding=(L%.1f,T%.1f,R%.1f,B%.1f)"),
				PlotWidth,
				PlotHeight,
				WidgetSize.X,
				WidgetSize.Y,
				GraphPadding.Left,
				GraphPadding.Top,
				GraphPadding.Right,
				GraphPadding.Bottom
			);
			LastSmallPlotLogSeconds = NowSeconds;
		}
		return BaseLayer + 2;
	}

	const int32 TotalSamples = DepthHistoryMillimeters.Num();
	const int32 MaxSamples = FMath::Max(2, MaxRenderedSamples);
	const int32 SamplesToDraw = FMath::Min(TotalSamples, MaxSamples);
	const int32 StartIndex = TotalSamples - SamplesToDraw;

	int32 MinDepth = TNumericLimits<int32>::Max();
	int32 MaxDepth = TNumericLimits<int32>::Lowest();
	for (int32 SampleIndex = StartIndex; SampleIndex < TotalSamples; ++SampleIndex)
	{
		const int32 Value = DepthHistoryMillimeters[SampleIndex];
		MinDepth = FMath::Min(MinDepth, Value);
		MaxDepth = FMath::Max(MaxDepth, Value);
	}

	if (MinDepth == MaxDepth)
	{
		MaxDepth = MinDepth + 1;
	}

	TArray<FVector2D> GraphPoints;
	GraphPoints.Reserve(SamplesToDraw);

	const float DepthRange = static_cast<float>(MaxDepth - MinDepth);
	for (int32 LocalIndex = 0; LocalIndex < SamplesToDraw; ++LocalIndex)
	{
		const int32 SampleValue = DepthHistoryMillimeters[StartIndex + LocalIndex];
		const float AlphaX = SamplesToDraw > 1
			? static_cast<float>(LocalIndex) / static_cast<float>(SamplesToDraw - 1)
			: 0.0f;
		const float NormalizedDepth = FMath::Clamp(
			(static_cast<float>(SampleValue - MinDepth)) / DepthRange,
			0.0f,
			1.0f
		);

		const float X = Left + AlphaX * PlotWidth;
		const float Y = Bottom - NormalizedDepth * PlotHeight;
		GraphPoints.Add(FVector2D(X, Y));
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		BaseLayer + 3,
		AllottedGeometry.ToPaintGeometry(),
		GraphPoints,
		ESlateDrawEffect::None,
		LineColor,
		false,
		LineThickness
	);

	if (GraphPoints.Num() > 0)
	{
		const FVector2D LatestPoint = GraphPoints.Last();
		const FVector2D MarkerTopLeft = LatestPoint - FVector2D(CurrentPointSize * 0.5f, CurrentPointSize * 0.5f);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			BaseLayer + 4,
			AllottedGeometry.ToPaintGeometry(MarkerTopLeft, FVector2D(CurrentPointSize, CurrentPointSize)),
			WhiteBrush,
			ESlateDrawEffect::None,
			CurrentPointColor
		);

		static double LastPaintOkLogSeconds = 0.0;
		const double NowSeconds = FPlatformTime::Seconds();
		if (NowSeconds - LastPaintOkLogSeconds > 1.0)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("DepthGraphWidget: NativePaint drew %d points (min=%d, max=%d, current=%d)"),
				GraphPoints.Num(),
				MinDepth,
				MaxDepth,
				CurrentDepthMillimeters
			);
			LastPaintOkLogSeconds = NowSeconds;
		}
	}

	return BaseLayer + 4;
}

