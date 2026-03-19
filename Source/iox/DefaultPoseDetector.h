// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ARTextures.h"
#include "BodyPoseManager.h"
#include "IPoseDetector.h"
#include "DefaultPoseDetector.generated.h"


UCLASS()
class IOX_API UDefaultPoseDetector : public UObject, public IIPoseDetector
{
	GENERATED_BODY()

public:	

    UDefaultPoseDetector();

    UPROPERTY(BlueprintReadOnly, Category="Pose")
    TObjectPtr<UBodyPoseManager> BodyPoseManager = nullptr;

    virtual void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pose")
    TObjectPtr<UTextureRenderTarget2D> RenderTarget = nullptr;

    virtual void PerformPoseDetectionOnFrame() const override;

    virtual const TArray<FPoseJoint>& GetDetectedJoints() const override;

private:
    mutable bool bSavedCameraFrameOnce = false;
    
    TArray<uint8> GetRenderTargetBytes() const;
    
    int32 GetRenderTargetWidth() const;
    
    int32 GetRenderTargetHeight() const;

    bool ValidateRawBytes(const TArray<uint8>& RawBytes, int32 Width, int32 Height, int32 Channels, bool bEmitStats) const;

    TArray<uint8> ConvertRGBAtoRGB(const TArray<uint8>& RGBABytes) const;

    void FlipImageRowsInPlace(TArray<uint8>& Bytes, int32 Width, int32 Height, int32 Channels) const;

    void SaveRenderTargetToPNG() const;
};
