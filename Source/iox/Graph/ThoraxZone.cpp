#include "Graph/ThoraxZone.h"
#include "Graph/GraphUtils.h"

void FThoraxZone::UpdateBounds(const FVector2D& InMin, const FVector2D& InMax)
{
    ZoneMinUV = InMin;
    ZoneMaxUV = InMax;
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

    return GraphMath::FindExtrema(XValues, YValues, Prominence, MinDistance);
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

float FThoraxZone::GetRespirationVolume(const FVector2D& FocalLength, const FVector2D& Resolution) const
{
    float Max = 0.0f;
    float Min = 0.0f;
    if (!GetLastMaxMinBreath(Max, Min))
    {
        return 0.0f;
    }

    float WidthMM = 0.0f;
    float HeightMM = 0.0f;
    if (!GetZoneDimensionsMM(FocalLength, Resolution, WidthMM, HeightMM))
    {
        return 0.0f;
    }

    // Breath Volume is Width * Height * DepthDifference
    return WidthMM * HeightMM * FMath::Abs(Max - Min);
}

bool FThoraxZone::GetZoneDimensionsMM(const FVector2D& FocalLength, const FVector2D& Resolution, float& OutWidthMM, float& OutHeightMM) const
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
