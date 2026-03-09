// Fill out your copyright notice in the Description page of Project Settings.


#include "PoseDetectionComponent.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UPoseDetectionComponent::UPoseDetectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
    

	// ...
}

// Called when the game starts
void UPoseDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    BodyPoseManager = NewObject<UBodyPoseManager>(this);
    bSavedCameraFrameOnce = false;
}


// Called every frame
void UPoseDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPoseDetectionComponent::PerformPoseDetectionOnFrame() const
{
    if (!BodyPoseManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformPoseDetectionOnFrame skipped: BodyPoseManager is null"));
        return;
    }

    const int32 Width = GetRenderTargetWidth();
    const int32 Height = GetRenderTargetHeight();

    if (Width <= 0 || Height <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformPoseDetectionOnFrame skipped: invalid render target size %d x %d"), Width, Height);
        return;
    }

    TArray<uint8> RawBytes = GetRenderTargetBytes();

    if (!bSavedCameraFrameOnce)
    {
        SaveRenderTargetToPNG();
        bSavedCameraFrameOnce = true;
        UE_LOG(LogTemp, Warning, TEXT("Saved first camera frame PNG (see full path in previous log line)"));
    }

    if (bPoseInputForceOpaqueAlpha)
    {
        for (int32 i = 0; i < RawBytes.Num(); i += 4)
        {
            if (bPoseInputForceOpaqueAlpha)
            {
                RawBytes[i + 3] = 255;
            }
        }
    }

    BodyPoseManager->PerformPoseDetection(RawBytes, Width, Height);
}

void UPoseDetectionComponent::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
    RenderTarget = InRenderTarget;
}

TArray<uint8> UPoseDetectionComponent::GetRenderTargetBytes() const
{
    if (!RenderTarget)
    {
        return TArray<uint8>();
    }
    
    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();

    if (!RTResource)
    {
        return TArray<uint8>();
    }

    TArray<FColor> PixelData;
    bool bReadSuccess = RTResource->ReadPixels(PixelData);

    if (!bReadSuccess || PixelData.Num() == 0)
    {
        return TArray<uint8>();
    }

    int32 Width  = RenderTarget->SizeX;
    int32 Height = RenderTarget->SizeY;

    TArray<uint8> RGBABytes;
    RGBABytes.SetNumUninitialized(PixelData.Num() * 4);

    for (int32 i = 0; i < PixelData.Num(); ++i)
    {
        const FColor& Pixel = PixelData[i];

        int32 DestIndex = i * 4;

        RGBABytes[DestIndex + 0] = Pixel.R;
        RGBABytes[DestIndex + 1] = Pixel.G;
        RGBABytes[DestIndex + 2] = Pixel.B;
        RGBABytes[DestIndex + 3] = Pixel.A;
    }

    return RGBABytes;
}

void UPoseDetectionComponent::SaveRenderTargetToPNG() const
{
    if (!RenderTarget) return;

    FTextureRenderTargetResource* RTResource =
        RenderTarget->GameThread_GetRenderTargetResource();

    if (!RTResource) return;

    TArray<FColor> PixelData;
    if (!RTResource->ReadPixels(PixelData) || PixelData.Num() == 0)
        return;

    const int32 Width  = RenderTarget->SizeX;
    const int32 Height = RenderTarget->SizeY;

    // Comprime in PNG con la nuova API
    TArray64<uint8> CompressedPNG;
    FImageUtils::PNGCompressImageArray(
        Width,
        Height,
        PixelData,
        CompressedPNG
    );

    const FString OutputDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("PoseDebug"));
    IFileManager::Get().MakeDirectory(*OutputDir, true);

    const FString FilePath = FPaths::Combine(OutputDir, TEXT("DebugRenderTarget.png"));
    const bool bSaved = FFileHelper::SaveArrayToFile(CompressedPNG, *FilePath);

    if (bSaved)
    {
        const FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);
        UE_LOG(LogTemp, Warning, TEXT("Saved PNG to: %s"), *FullPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to save PNG to: %s"), *FilePath);
    }
}

int32 UPoseDetectionComponent::GetRenderTargetWidth() const
{
    return RenderTarget ? RenderTarget->SizeX : 0;
}

int32 UPoseDetectionComponent::GetRenderTargetHeight() const
{
    return RenderTarget ? RenderTarget->SizeY : 0;
}
