// GraphMathLibrary.h
#pragma once

#include "CoreMinimal.h"

namespace GraphMath
{
    struct FBreathPoint
    {
        int32 Index;
        float X;
        float Y;
        bool bIsPeak;
    };

    TArray<FBreathPoint> FindExtrema(const TArray<float>& XValues, const TArray<float>& YValues, float Prominence, int32 MinDistance);
}   