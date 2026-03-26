#pragma once

#include "CoreMinimal.h"

namespace GraphUtils
{
    /**
     * Applies exponential moving average smoothing to an array of floats.
     * @param RawData The original data.
     * @param Alpha The smoothing factor (0 to 1, where 1 is no smoothing).
     * @return A new array with smoothed values.
     */
    inline TArray<float> SmoothArray(const TArray<float>& RawData, float Alpha)
    {
        TArray<float> SmoothedData;
        SmoothedData.Reserve(RawData.Num());

        float PreviousValue = RawData[0];
        for (float Value : RawData)
        {
            float SmoothedValue = FMath::Lerp(PreviousValue, Value, Alpha);
            SmoothedData.Add(SmoothedValue);
            PreviousValue = SmoothedValue;
        }

        return SmoothedData;
    }
}
