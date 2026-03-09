#include "BodyPoseManager.h"
#include "PoseDetectionBridge.h"
#include "ARBlueprintLibrary.h"
#include "ARPin.h"
#include "ARTrackable.h"

#define POSE_VISION_LOGS 0

#if POSE_VISION_LOGS
#define VISION_LOG(...) NSLog(__VA_ARGS__)
#else
#define VISION_LOG(...) do {} while (0)
#endif


int32 UBodyPoseManager::GetBodyCount_Implementation() const
{
    VISION_LOG(@"[Vision] GetBodyCount_Implementation");

    return 0;
}

void UBodyPoseManager::PerformPoseDetection(TArray<uint8> RawBytes, int Width, int Height)
{
    DetectedJoints = DetectHumanPoseFromRGBA(RawBytes.GetData(), Width, Height);
}
