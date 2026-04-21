// Fill out your copyright notice in the Description page of Project Settings.

#include "Camera/DepthSampler.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

bool UDepthSampler::CaptureFrame(
    TObjectPtr<UMaterialInstanceDynamic> DepthMat,
    TObjectPtr<UTexture> ConfidenceTexture,
    TObjectPtr<UTextureRenderTarget2D> CameraRT,
    bool bUseFloat32)
{
    bHasData = false;
    bHasConfidenceData = false;
    FramePixels.Empty();
    ConfidencePixels.Empty();

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

    // --- Capture Depth ---
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

    if (DebugRenderTarget)
    {
        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, DebugRenderTarget, DepthMat);
        FTextureRenderTargetResource* RTResource = DebugRenderTarget->GameThread_GetRenderTargetResource();
        if (RTResource)
        {
            FReadSurfaceDataFlags ReadFlags;
            ReadFlags.SetLinearToGamma(false);
            if (RTResource->ReadLinearColorPixels(FramePixels, ReadFlags) && FramePixels.Num() > 0)
            {
                bHasData = true;
            }
        }
    }

    // --- Capture Confidence ---
    if (ConfidenceTexture)
    {
        if (!ConfidenceRenderTarget
            || ConfidenceRenderTarget->SizeX != RTWidth
            || ConfidenceRenderTarget->SizeY != RTHeight)
        {
            // Using 16f to ensure we capture the raw values (0, 1, 2) without normalization issues if possible,
            // or at least with enough precision.
            ConfidenceRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
                this, RTWidth, RTHeight, ETextureRenderTargetFormat::RTF_RGBA8);

            if (ConfidenceRenderTarget)
            {
                ConfidenceRenderTarget->bForceLinearGamma = true;
                ConfidenceRenderTarget->UpdateResourceImmediate(false);
            }
        }

        if (ConfidenceRenderTarget)
        {
            UCanvas* Canvas = nullptr;
            FVector2D CanvasSize;
            FDrawToRenderTargetContext Context;

            UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, ConfidenceRenderTarget, Canvas, CanvasSize, Context);
            if (Canvas)
            {
                // Use BLEND_Opaque to ensure we get the exact values
                Canvas->K2_DrawTexture(ConfidenceTexture, FVector2D::ZeroVector, CanvasSize, FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::White, BLEND_Opaque);
            }
            UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);

            FTextureRenderTargetResource* ConfRTResource = ConfidenceRenderTarget->GameThread_GetRenderTargetResource();
            if (ConfRTResource)
            {
                FReadSurfaceDataFlags ReadFlags;
                ReadFlags.SetLinearToGamma(false);
                if (ConfRTResource->ReadLinearColorPixels(ConfidencePixels, ReadFlags) && ConfidencePixels.Num() > 0)
                {
                    bHasConfidenceData = true;
                }
            }
        }
    }

    return bHasData;
}

bool UDepthSampler::ComputeMeanInBoundsUV(
    const FVector2D& MinUV,
    const FVector2D& MaxUV,
    float& OutMeanDepthValue,
    int32& OutSampleCount,
    float& OutMinDepthValue,
    float& OutMaxDepthValue,
    float& OutDepthSampleConfidence,
    float MinConfidence) const
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
    float ConfSum  = 0.0f;
    int32 Count    = 0;
    float MinDepth = TNumericLimits<float>::Max();
    float MaxDepth = TNumericLimits<float>::Lowest();

    const bool bUseConfidence = bHasConfidenceData && (ConfidencePixels.Num() == FramePixels.Num());

    for (int32 Y = StartY; Y <= EndY; ++Y)
    {
        for (int32 X = StartX; X <= EndX; ++X)
        {
            const int32 PixelIdx = Y * Width + X;
            const FLinearColor& DepthPixel = FramePixels[PixelIdx];
            const float PixelDepth = DepthPixel.R;

            float PixelConfidence = 1.0f;
            if (bUseConfidence)
            {
                const float RawConf = ConfidencePixels[PixelIdx].R;
                
                // ARKit confidence levels are 0 (Low), 1 (Medium), 2 (High).
                // If the source texture is 8-bit (PF_G8), drawing it to a RenderTarget 
                // results in values like 0/255, 1/255, 2/255.
                // We remap these to 0.0, 0.5, 1.0 to match the expected 0-1 range.
                if (RawConf > 0.0f && RawConf < 0.05f) 
                {
                    PixelConfidence = FMath::Clamp(RawConf * 127.5f, 0.0f, 1.0f);
                }
                else
                {
                    PixelConfidence = RawConf;
                }
            }
            
            if (!FMath::IsFinite(PixelDepth) || PixelConfidence < MinConfidence)
            {
                continue;
            }

            DepthSum += PixelDepth;
            ConfSum  += PixelConfidence;
            MinDepth  = FMath::Min(MinDepth, PixelDepth);
            MaxDepth  = FMath::Max(MaxDepth, PixelDepth);
            ++Count;
        }
    }

    if (Count <= 0)
    {
        return false;
    }

    OutSampleCount           = Count;
    OutMeanDepthValue        = DepthSum / static_cast<float>(Count);
    OutMinDepthValue         = MinDepth;
    OutMaxDepthValue         = MaxDepth;
    OutDepthSampleConfidence = ConfSum / static_cast<float>(Count);

    return true;
}
