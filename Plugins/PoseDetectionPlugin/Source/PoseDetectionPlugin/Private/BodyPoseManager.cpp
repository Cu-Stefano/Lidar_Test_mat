#include "BodyPoseManager.h"
#include "PoseDetectionBridge.h"
#include "ARBlueprintLibrary.h"
#include "ARPin.h"
#include "ARTrackable.h"


int32 UBodyPoseManager::GetBodyCount_Implementation() const
{
    NSLog(@"[Vision] GetBodyCount_Implementation");

    return 0;
}

void UBodyPoseManager::PerformPoseDetection(TArray<uint8> RawBytes, int Width, int Height)
{
    DetectedJoints = DetectHumanPoseFromRGBA(RawBytes.GetData(), Width, Height);
}
