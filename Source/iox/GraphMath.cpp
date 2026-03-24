
// GraphMathLibrary.cpp
#include "GraphMath.h"

namespace GraphMath
{
    TArray<FBreathPoint> FindExtrema(
    const TArray<float>& XValues,
    const TArray<float>& YValues,
    float Prominence,      // % dell'ampiezza totale (es. 0.15 = 15%)
    int32 MinDistance)     // campioni minimi tra due estremi
{
    TArray<FBreathPoint> Result;

    if (YValues.Num() < 3 || YValues.Num() != XValues.Num())
        return Result;

    // Ampiezza totale per isolare le fluttuazioni microscopiche (noise)
    float YMin = YValues[0];
    float YMax = YValues[0];
    for (float Val : YValues)
    {
        YMin = FMath::Min(YMin, Val);
        YMax = FMath::Max(YMax, Val);
    }
    float AbsProminence = Prominence * (YMax - YMin);

    int32 Window = FMath::Max(1, MinDistance);
    int32 LastPeakIndex = -Window * 2;
    int32 LastValleyIndex = -Window * 2;

    for (int32 i = 0; i < YValues.Num(); i++)
    {
        int32 Left = FMath::Max(0, i - Window);
        int32 Right = FMath::Min(YValues.Num() - 1, i + Window);

        bool bIsPeak = true;
        bool bIsValley = true;

        for (int32 j = Left; j <= Right; j++)
        {
            if (j == i) continue;
            if (YValues[j] > YValues[i]) bIsPeak = false;
            if (YValues[j] < YValues[i]) bIsValley = false;
        }

        if (bIsPeak && (i - LastPeakIndex) >= Window)
        {
            // Verifica che il picco abbia sufficiente altezza (noise filter)
            float LocalMin = YValues[i];
            for (int32 j = Left; j <= Right; j++) LocalMin = FMath::Min(LocalMin, YValues[j]);
            if (FMath::Abs(YValues[i] - LocalMin) >= AbsProminence)
            {
                FBreathPoint Point;
                Point.Index   = i;
                Point.X       = XValues[i];
                Point.Y       = YValues[i];
                Point.bIsPeak = true;
                Result.Add(Point);
                LastPeakIndex = i;
            }
        }
        else if (bIsValley && (i - LastValleyIndex) >= Window)
        {
            // Verifica che la valle abbia sufficiente profondità (noise filter)
            float LocalMax = YValues[i];
            for (int32 j = Left; j <= Right; j++) LocalMax = FMath::Max(LocalMax, YValues[j]);
            if (FMath::Abs(LocalMax - YValues[i]) >= AbsProminence)
            {
                FBreathPoint Point;
                Point.Index   = i;
                Point.X       = XValues[i];
                Point.Y       = YValues[i];
                Point.bIsPeak = false;
                Result.Add(Point);
                LastValleyIndex = i;
            }
        }
    }

    return Result;
}
}

