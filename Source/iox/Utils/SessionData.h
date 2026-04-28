#pragma once

#include "CoreMinimal.h"

struct FBreathingAnnotation
{
    int32 Index = 0;
    FString Label;
};

struct FBreathingSessionData
{
    FString Title;
    FString Description;
    TArray<FString> Labels;
    TArray<float> Values;
    TArray<FBreathingAnnotation> Annotations;

    void Reset()
    {
        Labels.Empty();
        Values.Empty();
        Annotations.Empty();
    }
};