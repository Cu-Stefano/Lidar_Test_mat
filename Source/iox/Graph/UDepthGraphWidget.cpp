// Fill out your copyright notice in the Description page of Project Settings.


#include "Graph/UDepthGraphWidget.h"
#include "Graph/GraphMath.h"

#include "Fonts/SlateFontInfo.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "HAL/PlatformTime.h"
#include "Layout/Geometry.h"
#include "Rendering/SlateLayoutTransform.h"
#include "Math/NumericLimits.h"

void UDepthGraphWidget::SetGraphData(const TArray<float>& InDepthHistory, const TArray<FDateTime>& InTimeHistory, const TArray<float>& TotalVolumes, const float InCurrentDepth, const bool bInHasDepth)
{
	DepthHistory = InDepthHistory;
	TimeHistory = InTimeHistory;
	VolumeLabels = TotalVolumes;
	CurrentDepth = InCurrentDepth;
	bHasDepth = bInHasDepth;

	InvalidateLayoutAndVolatility();
}

int32 UDepthGraphWidget::NativePaint(
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

	if (WidgetSize.X <= 1.0f || WidgetSize.Y <= 1.0f)
	{
		static float LastTinySizeLogSeconds = 0.0;
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

	if (!bHasDepth || DepthHistory.Num() < 2)
	{
		static float LastNoDataPaintLogSeconds = 0.0;
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
		static float LastSmallPlotLogSeconds = 0.0;
		return BaseLayer + 2;
	}

	const int32 TotalSamples = DepthHistory.Num();
	const int32 MaxSamples = FMath::Max(2, MaxHistorySamples);
	const int32 SamplesToDraw = FMath::Min(TotalSamples, MaxSamples);
	const int32 StartIndex = TotalSamples - SamplesToDraw;

	// scale graph based on Max e Min value in a specific window of recent History
	float MinDepth = TNumericLimits<float>::Max();
	float MaxDepth = TNumericLimits<float>::Lowest();
	int32 ValidSamples = 0;

	// Use only the most recent N samples to determine the scaling range
	const int32 CalculationWindow = FMath::Clamp(SamplesForMinMaxCalculation, 2, TotalSamples);
	const int32 CalculationStartIndex = TotalSamples - CalculationWindow;

	for (int32 SampleIndex = CalculationStartIndex; SampleIndex < TotalSamples; ++SampleIndex)
	{
		const float Value = DepthHistory[SampleIndex];
		if (!FMath::IsFinite(Value))
		{
			continue;
		}
		MinDepth = FMath::Min(MinDepth, Value);
		MaxDepth = FMath::Max(MaxDepth, Value);
		++ValidSamples;
	}

	if (ValidSamples < 2)
	{
		// Fallback to clamp ranges if no valid samples
		MinDepth = MinYAxisClamp;
		MaxDepth = MaxYAxisClamp;
	}
	else
	{
		// Add padding and apply absolute clamps
		MinDepth = FMath::Max(MinYAxisClamp, MinDepth - FixedYRangePadding);
		MaxDepth = FMath::Min(MaxYAxisClamp, MaxDepth + FixedYRangePadding);
	}

	const float MidDepth = 0.5f * (MinDepth + MaxDepth);
	const FSlateFontInfo AxisFontInfo = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10);
	const FLinearColor AxisLabelColor(0.88f, 0.88f, 0.88f, 0.95f);
	const FLinearColor AxisGuideColor(0.35f, 0.35f, 0.35f, 0.50f);

	const float AxisTextX = GraphPadding.Left;
	const float AxisTickStartX = Left - YAxisTickLength;

	const auto DrawYAxisLabel = [&](const int32 Layer, const float Y, const float DepthValue)
	{
		const FString Label = FString::Printf(TEXT("%.0f"), DepthValue);
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

	DrawYAxisLabel(BaseLayer + 3, Top, MinDepth);
	DrawYAxisLabel(BaseLayer + 3, Top + 0.5f * PlotHeight, MidDepth);
	DrawYAxisLabel(BaseLayer + 3, Bottom, MaxDepth);

	const auto DrawXAxisLabel = [&](const int32 Layer, const float X, const FDateTime& Time)
	{
		const FString Label = Time.ToString(TEXT("%H:%M:%S"));
		const FVector2f LabelPosition(
			static_cast<float>(X - 30.0f),
			static_cast<float>(Bottom + 2.0f)
		);

		FSlateDrawElement::MakeText(
			OutDrawElements,
			Layer,
			AllottedGeometry.ToPaintGeometry(
				FVector2f(100.0f, 20.0f),
				FSlateLayoutTransform(LabelPosition)
			),
			Label,
			AxisFontInfo,
			ESlateDrawEffect::None,
			AxisLabelColor
		);
	};

	if (TimeHistory.Num() >= 2)
	{
		const int32 T_TotalSamples = TimeHistory.Num();
		const int32 T_SamplesToDraw = FMath::Min(T_TotalSamples, MaxSamples);
		const int32 T_StartIndex = T_TotalSamples - T_SamplesToDraw;

		// Left time label
		DrawXAxisLabel(BaseLayer + 3, Left + 30.0f, TimeHistory[T_StartIndex]);
		// Middle label
		DrawXAxisLabel(BaseLayer + 3, Left + 0.5f * PlotWidth, TimeHistory[T_StartIndex + T_SamplesToDraw / 2]);
		// Right time label
		DrawXAxisLabel(BaseLayer + 3, Right - 30.0f, TimeHistory.Last());
	}

	TArray<FVector2D> GraphPoints;
	GraphPoints.Reserve(SamplesToDraw);

	TArray<float> SubArray;
	SubArray.Reserve(SamplesToDraw);
	for (int32 i = 0; i < SamplesToDraw; ++i)
	{
		SubArray.Add(DepthHistory[StartIndex + i]);
	}

	const float SmoothingAlpha = FMath::Clamp(DepthSmoothingAlpha, 0.01f, 1.0f);
	if (bEnableDepthSmoothing)
	{
		SubArray = GraphMath::SmoothArray(SubArray, SmoothingAlpha);
	}
	TArray<float> ScreenXs;
	ScreenXs.Reserve(SamplesToDraw);

	const float DepthRange = MaxDepth - MinDepth;

	for (int32 LocalIndex = 0; LocalIndex < SamplesToDraw; ++LocalIndex)
	{
		const float SmoothedValue = SubArray[LocalIndex];

		const float AlphaX = SamplesToDraw > 1
			? static_cast<float>(LocalIndex) / static_cast<float>(SamplesToDraw - 1)
			: 0.0f;
		const float NormalizedDepth = FMath::Clamp(
			(SmoothedValue - MinDepth) / DepthRange,
			0.0f,
			1.0f
		);

		const float X = Left + AlphaX * PlotWidth;
		// Invert Y mapping: MinDepth (closer) at Top, MaxDepth (farther) at Bottom
		const float Y = Top + NormalizedDepth * PlotHeight;
		const FVector2D CurrentPoint(X, Y);
		GraphPoints.Add(CurrentPoint);

		ScreenXs.Add(X);
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

	// Drawing min and max point markers
	if (SubArray.Num() >= 3)
	{
		TArray<GraphMath::FBreathPoint> Extrema = GraphMath::FindExtrema(ScreenXs, SubArray, 0.05f, 30);

		const int32 HistoryLimit = FMath::Max(1, MaxExtremaHistory);
		for (int32 ExtremaIndex = 0; ExtremaIndex < Extrema.Num(); ++ExtremaIndex)
		{
			const GraphMath::FBreathPoint& Extreme = Extrema[ExtremaIndex];
			const int32 SampleIndex = StartIndex + Extreme.Index;
			if (!DepthHistory.IsValidIndex(SampleIndex))
			{
				continue;
			}

			FDepthGraphExtremaSample Snapshot;
			Snapshot.SampleIndex = SampleIndex;
			Snapshot.Depth = DepthHistory[SampleIndex];
			Snapshot.bIsPeak = Extreme.bIsPeak;
			Snapshot.MarkerColor = Extreme.bIsPeak ? FLinearColor::Green : FLinearColor::Red;

			if (ExtremaHistory.Num() == 0)
			{
				ExtremaHistory.Add(Snapshot);
				continue;
			}

			FDepthGraphExtremaSample& LastSample = ExtremaHistory.Last();
			if (LastSample.bIsPeak == Snapshot.bIsPeak)
			{
				if (Snapshot.SampleIndex >= LastSample.SampleIndex)
				{
					// Keep only the most recent extremum "live" while the graph grows.
					LastSample = Snapshot;
				}
			}
			else if (Snapshot.SampleIndex > LastSample.SampleIndex)
			{
				ExtremaHistory.Add(Snapshot);
			}
		}

		if (ExtremaHistory.Num() > HistoryLimit)
		{
			ExtremaHistory.RemoveAt(0, ExtremaHistory.Num() - HistoryLimit, EAllowShrinking::No);
		}

		for (FDepthGraphExtremaSample& Sample : ExtremaHistory)
		{
			if (Sample.SampleIndex < StartIndex || Sample.SampleIndex >= TotalSamples)
			{
				continue;
			}

			const int32 LocalSampleIndex = Sample.SampleIndex - StartIndex;
			if (!GraphPoints.IsValidIndex(LocalSampleIndex))
			{
				continue;
			}

			const float NormalizedDepth = FMath::Clamp((Sample.Depth - MinDepth) / DepthRange, 0.0f, 1.0f);
			Sample.ScreenPosition = FVector2D(GraphPoints[LocalSampleIndex].X, Top + NormalizedDepth * PlotHeight);
		}

		const float ExtremesSize = FMath::Max(5.0f, CurrentPointSize * 1.5f);
		const FSlateFontInfo VolumeFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 15);
		const FLinearColor TextColor = FLinearColor::White;
		const int32 VisiblePhaseCount = FMath::Max(0, Extrema.Num() - 1);
		const int32 PhaseOffset = FMath::Max(0, VolumeLabels.Num() - VisiblePhaseCount);

		for (int32 i = 0; i < Extrema.Num(); ++i)
		{
			const GraphMath::FBreathPoint& Extreme = Extrema[i];
			if (Extreme.Index >= 0 && Extreme.Index < GraphPoints.Num())
			{
				const FVector2D& ExtremePoint = GraphPoints[Extreme.Index];
				const FVector2D MarkerTopLeft = ExtremePoint - FVector2D(ExtremesSize * 0.5f, ExtremesSize * 0.5f);
				
				FLinearColor MarkerColor = Extreme.bIsPeak ? FLinearColor::Green : FLinearColor::Red;

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					BaseLayer + 5,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(ExtremesSize, ExtremesSize),
						FSlateLayoutTransform(FVector2f(static_cast<float>(MarkerTopLeft.X), static_cast<float>(MarkerTopLeft.Y)))
					),
					WhiteBrush,
					ESlateDrawEffect::None,
					MarkerColor
				);

				const int32 VolumePhaseIndex = PhaseOffset + i;
				if (bShowVolumeOnExtrema && i + 1 < Extrema.Num() && VolumeLabels.IsValidIndex(VolumePhaseIndex))
				{
					const GraphMath::FBreathPoint& NextExtreme = Extrema[i + 1];

					if (NextExtreme.Index >= 0 && NextExtreme.Index < GraphPoints.Num())
					{
						const FVector2D& LabelPoint = GraphPoints[NextExtreme.Index];
						const FString VolumeStr = FString::Printf(TEXT("%.2f"), (VolumeLabels[VolumePhaseIndex] / 1000000.0f));
						
						// Position from anchored extremum to avoid frame-to-frame offset flips.
						FVector2f TextOffset;
						if (NextExtreme.bIsPeak)
						{
							TextOffset = FVector2f(-20.0f, 20.0f);
						}
						else
						{
							TextOffset = FVector2f(-20.0f, -20.0f);
						}

						FSlateDrawElement::MakeText(
							OutDrawElements,
							BaseLayer + 6,
							AllottedGeometry.ToPaintGeometry(
								FVector2f(100.0f, 20.0f),
								FSlateLayoutTransform(FVector2f(static_cast<float>(LabelPoint.X), static_cast<float>(LabelPoint.Y)) + TextOffset)
							),
							VolumeStr,
							VolumeFont,
							ESlateDrawEffect::None,
							TextColor
						);
					}
				}
			}
		}
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

	}

	return BaseLayer + 4;
}

