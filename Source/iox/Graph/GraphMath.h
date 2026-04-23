// GraphMathLibrary.h
#pragma once

#include "CoreMinimal.h"

namespace GraphMath
{
    struct FBreathPoint
    {
        int32 Index = -1;
        float X = 0.0f;
        float Y = 0.0f;
        bool bIsPeak = false;
    };

    TArray<FBreathPoint> FindExtrema(const TArray<float>& XValues, const TArray<float>& YValues, float Prominence, int32 MinDistance);

    TArray<float> SmoothArray(const TArray<float>& RawData, float Alpha);
}