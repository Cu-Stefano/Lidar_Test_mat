#include "Graph/ThoraxZone.h"
#include "Graph/GraphUtils.h"

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
    return GraphUtils::SmoothArray(DepthHistory, Alpha);
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

        const float MeanDepth = (Section.StartPoint.Y + Section.EndPoint.Y) / 2.0f;
        const float UVWidth = FMath::Abs(ZoneMaxUV.X - ZoneMinUV.X);
        const float UVHeight = FMath::Abs(ZoneMaxUV.Y - ZoneMinUV.Y);
        
        if (FocalLength.X > 0 && FocalLength.Y > 0 && Resolution.X > 0 && Resolution.Y > 0)
        {
            const float WidthMM = (UVWidth * Resolution.X * MeanDepth) / FocalLength.X;
            const float HeightMM = (UVHeight * Resolution.Y * MeanDepth) / FocalLength.Y;
            Section.Volume = WidthMM * HeightMM * FMath::Abs(Section.StartPoint.Y - Section.EndPoint.Y);
        }
        else
        {
            Section.Volume = 0.0f;
        }

        BreathSections.Add(Section);
    }

    return res;
}

bool FThoraxZone::GetLastMaxMinBreath(float& OutMax, float& OutMin) const
{
    TArray<GraphMath::FBreathPoint> Extrema = ComputeExtrema();
    if (Extrema.Num() < 4) 
    {
        OutMax = 0.0f;
        OutMin = 0.0f;
        return false;
    }
    
    const float FirstValue = Extrema[Extrema.Num() - 3].Y;
    const float SecondValue = Extrema[Extrema.Num() - 2].Y;
    
    OutMax = FMath::Max(FirstValue, SecondValue);
    OutMin = FMath::Min(FirstValue, SecondValue);
    return true;
}

float FThoraxZone::GetRespirationVolume() const
{
    float Max = 0.0f;
    float Min = 0.0f;
    if (!GetLastMaxMinBreath(Max, Min))
    {
        return 0.0f;
    }

    float WidthMM = 0.0f;
    float HeightMM = 0.0f;
    if (!GetZoneDimensionsMM(WidthMM, HeightMM))
    {
        return 0.0f;
    }

    // Breath Volume is Width * Height * DepthDifference
    return WidthMM * HeightMM * FMath::Abs(Max - Min);
}

bool FThoraxZone::GetZoneDimensionsMM(float& OutWidthMM, float& OutHeightMM) const
{
    if (FocalLength.X <= 0.0f || FocalLength.Y <= 0.0f || Resolution.X <= 0.0f || Resolution.Y <= 0.0f)
    {
        return false;
    }

    float Max = 0.0f;
    float Min = 0.0f;
    if (!GetLastMaxMinBreath(Max, Min))
    {
        return false;
    }

    const float MeanDepth = (Max + Min) / 2.0f;
    const float UVWidth = FMath::Abs(ZoneMaxUV.X - ZoneMinUV.X);
    const float UVHeight = FMath::Abs(ZoneMaxUV.Y - ZoneMinUV.Y);

    OutWidthMM = (UVWidth * Resolution.X * MeanDepth) / FocalLength.X;
    OutHeightMM = (UVHeight * Resolution.Y * MeanDepth) / FocalLength.Y;

    return true;
}

GraphExtr::FBreathSection FThoraxZone::GetBreathSectionAtIndex(const int32 Index) const
{
    if (BreathSections.IsValidIndex(Index))
    {
        return BreathSections[Index];
    }
    return GraphExtr::FBreathSection();
}

TArray<GraphExtr::FBreathSection> FThoraxZone::GetBreathSections() const
{
    return BreathSections;
}
