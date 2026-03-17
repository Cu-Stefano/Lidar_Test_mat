// Fill out your copyright notice in the Description page of Project Settings.


#include "UDepthGraphWidget.h"

#include "Fonts/SlateFontInfo.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UUDepthGraphWidget::SetGraphData(const TArray<int32>& InDepthHistoryMillimeters, const int32 InCurrentDepthMillimeters, const bool bInHasDepth)
{
	DepthHistoryMillimeters = InDepthHistoryMillimeters;
	CurrentDepthMillimeters = InCurrentDepthMillimeters;
	bHasDepth = bInHasDepth;

	const double NowSeconds = FPlatformTime::Seconds();
	if (bHasDepth)
	{
		RecentDepthMillimeters.Add(CurrentDepthMillimeters);
	}

	const int32 SampleWindow = FMath::Max(2, RecentRedExtremsSampleWindow);
	while (RecentDepthMillimeters.Num() > SampleWindow)
	{
		RecentDepthMillimeters.RemoveAt(0, 1, false);
	}

	{
		static double LastSetGraphDataLogSeconds = 0.0;
		if (NowSeconds - LastSetGraphDataLogSeconds > 0.5)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("DepthGraphWidget: SetGraphData history=%d current=%d hasDepth=%s recent=%d"),
				DepthHistoryMillimeters.Num(),
				CurrentDepthMillimeters,
				bHasDepth ? TEXT("true") : TEXT("false"),
				RecentDepthMillimeters.Num()
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

	// Draw background
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	const FPaintGeometry FullPaintGeometry = AllottedGeometry.ToPaintGeometry(
		FVector2f(static_cast<float>(WidgetSize.X), static_cast<float>(WidgetSize.Y)),
		FSlateLayoutTransform()
	);

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
			FullPaintGeometry,
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

	const float YAxisLabelAreaWidth = 52.0f;
	const float YAxisTickLength = 5.0f;
	const float Left = GraphPadding.Left + YAxisLabelAreaWidth;
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
	const int32 MaxSamples = FMath::Max(2, MaxHistorySamples);
	const int32 SamplesToDraw = FMath::Min(TotalSamples, MaxSamples);
	const int32 StartIndex = TotalSamples - SamplesToDraw;

	// scale graph based on Max e Min value
	int32 MinDepth = TNumericLimits<int32>::Max();
	int32 MaxDepth = TNumericLimits<int32>::Lowest();
	for (int32 SampleIndex = StartIndex; SampleIndex < TotalSamples; ++SampleIndex)
	{
		const int32 Value = DepthHistoryMillimeters[SampleIndex];
		MinDepth = FMath::Min(MinDepth, Value);
		MaxDepth = FMath::Max(MaxDepth, Value);
	}

	// add padding to top and bottom (max Y e min Y) of graph
	MinDepth = FMath::Max(0, MinDepth - FixedYRangePaddingMillimeters);
	MaxDepth = MaxDepth + FixedYRangePaddingMillimeters;
	if (MinDepth == MaxDepth)
	{
		MaxDepth = MinDepth + 1;
	}

	const int32 MidDepth = (MinDepth + MaxDepth) / 2;
	const FSlateFontInfo AxisFontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
	const FLinearColor AxisLabelColor(0.88f, 0.88f, 0.88f, 0.95f);
	const FLinearColor AxisGuideColor(0.35f, 0.35f, 0.35f, 0.50f);

	const float AxisTextX = GraphPadding.Left;
	const float AxisTickStartX = Left - YAxisTickLength;

	const auto DrawYAxisLabel = [&](const int32 Layer, const float Y, const int32 DepthMm)
	{
		const FString Label = FString::Printf(TEXT("%d"), DepthMm);
		const FVector2f LabelPosition(
			static_cast<float>(AxisTextX),
			static_cast<float>(Y - 7.0f)
		);

		FSlateDrawElement::MakeText(
			OutDrawElements,
			Layer,
			AllottedGeometry.ToPaintGeometry(
				FVector2f(static_cast<float>(WidgetSize.X), static_cast<float>(WidgetSize.Y)),
				FSlateLayoutTransform(LabelPosition)
			),
			Label,
			AxisFontInfo,
			ESlateDrawEffect::None,
			AxisLabelColor
		);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			Layer,
			FullPaintGeometry,
			{
				FVector2D(AxisTickStartX, Y),
				FVector2D(Left, Y),
				FVector2D(Right, Y)
			},
			ESlateDrawEffect::None,
			AxisGuideColor,
			false,
			1.0f
		);
	};

	DrawYAxisLabel(BaseLayer + 3, Top, MaxDepth);
	DrawYAxisLabel(BaseLayer + 3, Top + 0.5f * PlotHeight, MidDepth);
	DrawYAxisLabel(BaseLayer + 3, Bottom, MinDepth);

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
		FullPaintGeometry,
		GraphPoints,
		ESlateDrawEffect::None,
		LineColor,
		false,
		LineThickness
	);

	if (RecentDepthMillimeters.Num() > 0)
	{
		int32 RecentMinDepth = TNumericLimits<int32>::Max();
		int32 RecentMaxDepth = TNumericLimits<int32>::Lowest();
		for (const int32 DepthMm : RecentDepthMillimeters)
		{
			RecentMinDepth = FMath::Min(RecentMinDepth, DepthMm);
			RecentMaxDepth = FMath::Max(RecentMaxDepth, DepthMm);
		}

		const auto DepthToPlotY = [&](const int32 DepthMm)
		{
			const float NormalizedDepth = FMath::Clamp(
				(static_cast<float>(DepthMm - MinDepth)) / DepthRange,
				0.0f,
				1.0f
			);
			return Bottom - NormalizedDepth * PlotHeight;
		};

		const auto DrawRedExtremsMarker = [&](const float Y)
		{
			const float MarkerHalf = RecentRedExtremsMarkerSize * 0.5f;
			const float MarkerCenterX = Right - MarkerHalf - 1.0f;

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				BaseLayer + 4,
				FullPaintGeometry,
				{
					FVector2D(MarkerCenterX - MarkerHalf, Y),
					FVector2D(MarkerCenterX + MarkerHalf, Y)
				},
				ESlateDrawEffect::None,
				RecentRedExtremsColor,
				false,
				2.0f
			);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				BaseLayer + 4,
				AllottedGeometry.ToPaintGeometry(
					FVector2f(3.0f, 3.0f),
					FSlateLayoutTransform(
						FVector2f(
							static_cast<float>(MarkerCenterX - 1.5f),
							static_cast<float>(Y - 1.5f)
						)
					)
				),
				WhiteBrush,
				ESlateDrawEffect::None,
				RecentRedExtremsColor
			);
		};

		DrawRedExtremsMarker(DepthToPlotY(RecentMaxDepth));
		DrawRedExtremsMarker(DepthToPlotY(RecentMinDepth));
	}

	if (GraphPoints.Num() > 0)
	{
		const FVector2D LatestPoint = GraphPoints.Last();
		const FVector2D MarkerTopLeft = LatestPoint - FVector2D(CurrentPointSize * 0.5f, CurrentPointSize * 0.5f);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			BaseLayer + 4,
			AllottedGeometry.ToPaintGeometry(
				FVector2f(CurrentPointSize, CurrentPointSize),
				FSlateLayoutTransform(
					FVector2f(
						static_cast<float>(MarkerTopLeft.X),
						static_cast<float>(MarkerTopLeft.Y)
					)
				)
			),
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

