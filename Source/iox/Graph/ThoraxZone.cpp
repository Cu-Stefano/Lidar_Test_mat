#include "Graph/ThoraxZone.h"


void FThoraxZone::UpdateBounds(const FVector2D& InMin, const FVector2D& InMax, const FVector2D& InFocalLength, const FVector2D& InResolution)
{
    ZoneMinUV = InMin;
    ZoneMaxUV = InMax;
    FocalLength = InFocalLength;
    Resolution = InResolution;
}

FVector2D FThoraxZone::GetZoneMinUV() const
{
    return ZoneMinUV;
}

FVector2D FThoraxZone::GetZoneMaxUV() const
{
    return ZoneMaxUV;
}

float FThoraxZone::GetLastDepth() const
{
    return LastDepth;
}

const TArray<float>& FThoraxZone::GetDepthHistory() const
{
    return DepthHistory;
}

void FThoraxZone::AddDepthSample(float Depth, int32 MaxSamples)
{
    if (!FMath::IsFinite(Depth)) 
    {
        return;
    }
    
    LastDepth = Depth;
    DepthHistory.Add(Depth);

    if (DepthHistory.Num() > MaxSamples)
    {
        DepthHistory.RemoveAt(0, DepthHistory.Num() - MaxSamples, EAllowShrinking::No);
    }
}

TArray<float> FThoraxZone::GetSmoothedHistory(float Alpha) const
{
    return GraphMath::SmoothArray(DepthHistory, Alpha);
}

TArray<GraphMath::FBreathPoint> FThoraxZone::ComputeExtrema(float SmoothingAlpha, float Prominence, int32 MinDistance) const
{
    if (DepthHistory.Num() < 3) 
    {
        return {};
    }

    const TArray<float> YValues = GetSmoothedHistory(SmoothingAlpha);
    
    TArray<float> XValues;
    XValues.SetNum(YValues.Num());
    for (int32 i = 0; i < YValues.Num(); ++i) 
    { 
        XValues[i] = static_cast<float>(i); 
    }

    TArray<GraphMath::FBreathPoint> res = GraphMath::FindExtrema(XValues, YValues, Prominence, MinDistance);
    

    BreathSections.Empty();
    if (res.Num() < 2) return res;

    for (int32 i = 0; i < res.Num() - 1; ++i)
    {
        GraphExtr::FBreathSection Section;
        Section.StartPoint = res[i];
        Section.EndPoint = res[i + 1];

        const bool bHasValidDepthPair = DepthHistory.IsValidIndex(Section.StartPoint.Index) && DepthHistory.IsValidIndex(Section.EndPoint.Index);
        const float StartDepth = bHasValidDepthPair ? DepthHistory[Section.StartPoint.Index] : 0.0f;
        const float EndDepth = bHasValidDepthPair ? DepthHistory[Section.EndPoint.Index] : 0.0f;

        const float MeanDepth = bHasValidDepthPair
            ? (StartDepth + EndDepth) / 2.0f
            : 0.0f;
            
        const float UVWidth = FMath::Abs(ZoneMaxUV.X - ZoneMinUV.X);
        const float UVHeight = FMath::Abs(ZoneMaxUV.Y - ZoneMinUV.Y);
        
        if (FocalLength.X > 0 && FocalLength.Y > 0 && Resolution.X > 0 && Resolution.Y > 0)
        {
            const float WidthMM = (UVWidth * Resolution.X * MeanDepth) / FocalLength.X;
            const float HeightMM = (UVHeight * Resolution.Y * MeanDepth) / FocalLength.Y;
            const float DepthDelta = bHasValidDepthPair ? FMath::Abs(StartDepth - EndDepth) : 0.0f;
            Section.Volume = WidthMM * HeightMM * DepthDelta;
        }
        else
        {
            Section.Volume = 0.0f;
        }

        BreathSections.Add(Section);
    }

    return res;
}

float FThoraxZone::GetVolumeBetweenIndexes(int32 StartIndex, int32 EndIndex) const
{
    if (!DepthHistory.IsValidIndex(StartIndex) || !DepthHistory.IsValidIndex(EndIndex))
    {
        return 0.0f;
    }

    const float StartDepth = DepthHistory[StartIndex];
    const float EndDepth = DepthHistory[EndIndex];
    if (!FMath::IsFinite(StartDepth) || !FMath::IsFinite(EndDepth) || StartDepth <= 0.0f || EndDepth <= 0.0f)
    {
        return 0.0f;
    }

    if (FocalLength.X <= 0.0f || FocalLength.Y <= 0.0f || Resolution.X <= 0.0f || Resolution.Y <= 0.0f)
    {
        return 0.0f;
    }

    const float MeanDepth = 0.5f * (StartDepth + EndDepth);
    const float UVWidth = FMath::Abs(ZoneMaxUV.X - ZoneMinUV.X);
    const float UVHeight = FMath::Abs(ZoneMaxUV.Y - ZoneMinUV.Y);
    const float WidthMM = (UVWidth * Resolution.X * MeanDepth) / FocalLength.X;
    const float HeightMM = (UVHeight * Resolution.Y * MeanDepth) / FocalLength.Y;
    const float DepthDelta = FMath::Abs(EndDepth - StartDepth);

    return WidthMM * HeightMM * DepthDelta;
}

