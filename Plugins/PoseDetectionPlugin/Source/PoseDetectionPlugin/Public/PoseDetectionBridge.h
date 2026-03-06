#pragma once

#include "CoreMinimal.h"
#include "PoseDetectionBridge.generated.h"

USTRUCT(BlueprintType)
struct FPoseJoint
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly)
    FString Name;

    UPROPERTY(BlueprintReadOnly)
    float X = 0.f;

    UPROPERTY(BlueprintReadOnly)
    float Y = 0.f;

    UPROPERTY(BlueprintReadOnly)
    float Confidence = 0.f;
};

void DetectHumanPoseFromRGBA_Debug();
TArray<FPoseJoint> DetectHumanPoseFromRGBA(const uint8_t* RGBAData, int Width, int Height);
