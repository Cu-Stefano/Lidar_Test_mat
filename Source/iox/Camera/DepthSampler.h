// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DepthSampler.generated.h"

class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;

UCLASS()
class IOX_API UDepthSampler : public UObject
{
    GENERATED_BODY()

public:
    bool CaptureFrame(
        TObjectPtr<UMaterialInstanceDynamic> DepthMat,
        TObjectPtr<UTextureRenderTarget2D> CameraRT,
        bool bUseFloat32);

    bool ComputeMeanInBoundsUV(
        const FVector2D& MinUV,
        const FVector2D& MaxUV,
        float& OutMeanDepthValue,
        int32& OutSampleCount,
        float& OutMinDepthValue,
        float& OutMaxDepthValue,
        float& OutDepthSampleConfidence) const;

private:
    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> DebugRenderTarget = nullptr;

    UPROPERTY(Transient)
    TArray<FLinearColor> FramePixels;

    int32 RTWidth = 0;
    int32 RTHeight = 0;
    bool bHasData = false;
};
