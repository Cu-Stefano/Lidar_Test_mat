#pragma once

#include "CoreMinimal.h"
#include "PoseDetectionBridge.h"
#include "BodyPoseInterface.h"
#include "BodyPoseManager.generated.h"


UCLASS(Blueprintable, BlueprintType)
class POSEDETECTIONPLUGIN_API UBodyPoseManager: public UObject, public IBodyPoseInterface
{
    GENERATED_BODY()

public:
    
    virtual int32 GetBodyCount_Implementation() const override;
    
    virtual void PerformPoseDetection(TArray<uint8> RawBytes, int Width, int Height);
    
    UPROPERTY(BlueprintReadOnly, Category="Pose")
    TArray<FPoseJoint> DetectedJoints;
    
private:
    
};
