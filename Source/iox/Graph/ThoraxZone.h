#pragma once

#include "CoreMinimal.h"
#include "Graph/GraphMath.h"
namespace GraphExtr{
    struct FBreathSection
    {
        GraphMath::FBreathPoint StartPoint;
        GraphMath::FBreathPoint EndPoint;
        float Volume = 0.0f;
    };  
}

class FThoraxZone
{
public:
    FThoraxZone() = default;

    void UpdateBounds(const FVector2D& InMin, const FVector2D& InMax, const FVector2D& InFocalLength, const FVector2D& InResolution);

    // Getters
    FVector2D GetZoneMinUV() const;
    FVector2D GetZoneMaxUV() const;
    float GetLastDepth() const;
    const TArray<float>& GetDepthHistory() const;

    void AddDepthSample(float Depth, int32 MaxSamples);

    TArray<float> GetSmoothedHistory(float Alpha) const;

    TArray<GraphMath::FBreathPoint> ComputeExtrema(float SmoothingAlpha = 0.15f, float Prominence = 0.05f, int32 MinDistance = 30) const;

    bool GetLastMaxMinBreath(float& OutMax, float& OutMin) const;
    
    float GetRespirationVolume() const;
    float GetVolumeBetweenIndexes(int32 StartIndex, int32 EndIndex) const;
    bool GetZoneDimensionsMM(float& OutWidthMM, float& OutHeightMM) const;
    
    GraphExtr::FBreathSection GetBreathSectionAtIndex(const int32 Index) const;
    
    TArray<GraphExtr::FBreathSection> GetBreathSections() const;
private:
    FVector2D FocalLength = FVector2D::ZeroVector;
    FVector2D Resolution = FVector2D::ZeroVector;
    FVector2D ZoneMinUV = FVector2D::ZeroVector;
    FVector2D ZoneMaxUV = FVector2D::ZeroVector;
    float edgeMm = 0.0f;
    float LastDepth = 0.0f;
    TArray<float> DepthHistory;
    mutable TArray<GraphExtr::FBreathSection> BreathSections;
};
