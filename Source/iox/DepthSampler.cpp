// Fill out your copyright notice in the Description page of Project Settings.

#include "DepthSampler.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

bool UDepthSampler::CaptureFrame(
    UMaterialInstanceDynamic* DepthMat,
    UTextureRenderTarget2D* CameraRT,
    bool bUseFloat32)
{
    bHasData = false;
    FramePixels.Empty();

    if (!DepthMat)
    {
        return false;
    }

    // Default resolution; override from camera render target if available.
    RTWidth  = 256;
    RTHeight = 256;

    if (CameraRT && CameraRT->SizeX > 0 && CameraRT->SizeY > 0)
    {
        RTWidth  = CameraRT->SizeX;
        RTHeight = CameraRT->SizeY;
    }

    // Create or resize the internal debug render target when needed.
    if (!DebugRenderTarget
        || DebugRenderTarget->SizeX != RTWidth
        || DebugRenderTarget->SizeY != RTHeight)
    {
        const ETextureRenderTargetFormat Fmt = bUseFloat32
            ? ETextureRenderTargetFormat::RTF_RGBA32f
            : ETextureRenderTargetFormat::RTF_RGBA16f;

        DebugRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
            this, RTWidth, RTHeight, Fmt);

        if (DebugRenderTarget)
        {
            DebugRenderTarget->bForceLinearGamma = true;
            DebugRenderTarget->UpdateResourceImmediate(false);
        }
    }

    if (!DebugRenderTarget)
    {
        return false;
    }

    UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, DebugRenderTarget, DepthMat);

    FTextureRenderTargetResource* RTResource = DebugRenderTarget->GameThread_GetRenderTargetResource();
    if (!RTResource)
    {
        return false;
    }

    FReadSurfaceDataFlags ReadFlags;
    ReadFlags.SetLinearToGamma(false);
    if (!RTResource->ReadLinearColorPixels(FramePixels, ReadFlags) || FramePixels.Num() == 0)
    {
        return false;
    }

    bHasData = true;
    return true;
}

bool UDepthSampler::ComputeMeanInBoundsUV(
    const FVector2D& MinUV,
    const FVector2D& MaxUV,
    float& OutMeanDepthValue,
    int32& OutSampleCount,
    float& OutMinDepthValue,
    float& OutMaxDepthValue,
    float& OutDepthSampleConfidence) const
{
    if (!bHasData || FramePixels.Num() == 0)
    {
        return false;
    }

    const int32 Width  = RTWidth;
    const int32 Height = RTHeight;

    const int32 StartX = FMath::Clamp(FMath::RoundToInt(FMath::Min(MinUV.X, MaxUV.X) * static_cast<float>(Width  - 1)), 0, Width  - 1);
    const int32 EndX   = FMath::Clamp(FMath::RoundToInt(FMath::Max(MinUV.X, MaxUV.X) * static_cast<float>(Width  - 1)), 0, Width  - 1);
    const int32 StartY = FMath::Clamp(FMath::RoundToInt(FMath::Min(MinUV.Y, MaxUV.Y) * static_cast<float>(Height - 1)), 0, Height - 1);
    const int32 EndY   = FMath::Clamp(FMath::RoundToInt(FMath::Max(MinUV.Y, MaxUV.Y) * static_cast<float>(Height - 1)), 0, Height - 1);

    float DepthSum = 0.0f;
    int32 Count    = 0;
    float MinDepth = TNumericLimits<float>::Max();
    float MaxDepth = TNumericLimits<float>::Lowest();

    for (int32 Y = StartY; Y <= EndY; ++Y)
    {
        for (int32 X = StartX; X <= EndX; ++X)
        {
            const FLinearColor& Pixel = FramePixels[Y * Width + X];
            const float PixelDepth   = Pixel.R;

            if (!FMath::IsFinite(PixelDepth))
            {
                continue;
            }

            DepthSum += PixelDepth;
            MinDepth  = FMath::Min(MinDepth, PixelDepth);
            MaxDepth  = FMath::Max(MaxDepth, PixelDepth);
            ++Count;
        }
    }

    if (Count <= 0)
    {
        return false;
    }

    OutSampleCount          = Count;
    OutMeanDepthValue       = DepthSum / static_cast<float>(Count);
    OutMinDepthValue        = MinDepth;
    OutMaxDepthValue        = MaxDepth;
    OutDepthSampleConfidence = 1.0f; // Simplified

    return true;
}
