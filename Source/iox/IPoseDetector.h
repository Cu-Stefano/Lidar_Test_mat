// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoseDetectionBridge.h"
#include "IPoseDetector.generated.h"

class UTextureRenderTarget2D;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIPoseDetector : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface per qualsiasi componente di pose detection.
 * Attualmente implementata da UDefaultPoseDetector (via BodyPoseManager).
 */
class IOX_API IIPoseDetector
{
    GENERATED_BODY()

public:
    virtual void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget) = 0;
    virtual void PerformPoseDetectionOnFrame() const = 0;
    virtual const TArray<FPoseJoint>& GetDetectedJoints() const = 0;
};
