#pragma once

#include "CoreMinimal.h"
#include "Graph/GraphMath.h"

class FThoraxZone
{
public:
    FThoraxZone() = default;

    void UpdateBounds(const FVector2D& InMin, const FVector2D& InMax);

    // Getters
    FVector2D GetZoneMinUV() const;
    FVector2D GetZoneMaxUV() const;
    float GetLastDepth() const;
    const TArray<float>& GetDepthHistory() const;

    void AddDepthSample(float Depth, int32 MaxSamples);

    TArray<float> GetSmoothedHistory(float Alpha) const;

    TArray<GraphMath::FBreathPoint> ComputeExtrema(float SmoothingAlpha = 0.15f, float Prominence = 0.05f, int32 MinDistance = 30) const;

    bool GetLastMaxMinBreath(float& OutMax, float& OutMin) const;
    
    float GetRespirationVolume(const FVector2D& FocalLength, const FVector2D& Resolution) const;
    bool GetZoneDimensionsMM(const FVector2D& FocalLength, const FVector2D& Resolution, float& OutWidthMM, float& OutHeightMM) const;
    
private:
    FVector2D ZoneMinUV = FVector2D::ZeroVector;
    FVector2D ZoneMaxUV = FVector2D::ZeroVector;
    float edgeMm = 0.0f;
    float LastDepth = 0.0f;
    TArray<float> DepthHistory;
};
