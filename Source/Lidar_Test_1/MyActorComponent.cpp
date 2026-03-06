// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActorComponent.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

#include "Engine/TextureRenderTarget2D.h"


// Sets default values for this component's properties
UMyActorComponent::UMyActorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UMyActorComponent::BeginPlay()
{
    Super::BeginPlay();

    BodyPoseManager = NewObject<UBodyPoseManager>(this);
    bSavedCameraFrameOnce = false;
}


// Called every frame
void UMyActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMyActorComponent::OnTextureChange() const
{
    if (!BodyPoseManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnTextureChange skipped: BodyPoseManager is null"));
        return;
    }

    const int32 Width = GetRenderTargetWidth();
    const int32 Height = GetRenderTargetHeight();

    if (Width <= 0 || Height <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnTextureChange skipped: invalid render target size %d x %d"), Width, Height);
        return;
    }

    TArray<uint8> RawBytes = GetRenderTargetBytes();

    constexpr int32 SourceChannels = 4;
    if (!ValidateRawBytes(RawBytes, Width, Height, SourceChannels, false))
    {
        return;
    }

    if (!bSavedCameraFrameOnce)
    {
        SaveRenderTargetToPNG();
        bSavedCameraFrameOnce = true;
        UE_LOG(LogTemp, Warning, TEXT("Saved first camera frame PNG (see full path in previous log line)"));
    }

    if (bPoseInputSwapRedBlue || bPoseInputForceOpaqueAlpha)
    {
        for (int32 i = 0; i < RawBytes.Num(); i += 4)
        {
            if (bPoseInputSwapRedBlue)
            {
                Swap(RawBytes[i + 0], RawBytes[i + 2]);
            }

            if (bPoseInputForceOpaqueAlpha)
            {
                RawBytes[i + 3] = 255;
            }
        }
    }

    int32 ChannelsToSend = SourceChannels;
    if (bPoseInputDropAlpha)
    {
        RawBytes = ConvertRGBAtoRGB(RawBytes);
        ChannelsToSend = 3;
    }

    if (bPoseInputFlipY)
    {
        FlipImageRowsInPlace(RawBytes, Width, Height, ChannelsToSend);
    }

    if (!ValidateRawBytes(RawBytes, Width, Height, ChannelsToSend, bLogPoseInputStats))
    {
        return;
    }

    BodyPoseManager->PerformPoseDetection(RawBytes, Width, Height);
}

void UMyActorComponent::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
    RenderTarget = InRenderTarget;
}

TArray<uint8> UMyActorComponent::GetRenderTargetBytes() const
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

void UMyActorComponent::SaveRenderTargetToPNG() const
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

int32 UMyActorComponent::GetRenderTargetWidth() const
{
    return RenderTarget ? RenderTarget->SizeX : 0;
}

int32 UMyActorComponent::GetRenderTargetHeight() const
{
    return RenderTarget ? RenderTarget->SizeY : 0;
}

bool UMyActorComponent::ValidateRawBytes(const TArray<uint8>& RawBytes, int32 Width, int32 Height, int32 Channels, bool bEmitStats) const
{
    if (Channels <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid channel count: %d"), Channels);
        return false;
    }

    const int64 ExpectedNum = static_cast<int64>(Width) * static_cast<int64>(Height) * static_cast<int64>(Channels);
    if (ExpectedNum <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid expected buffer size for %d x %d x %d"), Width, Height, Channels);
        return false;
    }

    if (RawBytes.Num() != ExpectedNum)
    {
        UE_LOG(LogTemp, Warning, TEXT("Pose input size mismatch: got %d bytes, expected %lld for %d x %d x %d"), RawBytes.Num(), ExpectedNum, Width, Height, Channels);
        return false;
    }

    if (bEmitStats && RawBytes.Num() >= Channels)
    {
        int32 MinValue = 255;
        int32 MaxValue = 0;
        uint64 Sum = 0;

        for (uint8 Value : RawBytes)
        {
            MinValue = FMath::Min(MinValue, static_cast<int32>(Value));
            MaxValue = FMath::Max(MaxValue, static_cast<int32>(Value));
            Sum += Value;
        }

        const double Mean = static_cast<double>(Sum) / static_cast<double>(RawBytes.Num());
        UE_LOG(
            LogTemp,
            Log,
            TEXT("Pose input stats: bytes=%d channels=%d min=%d max=%d mean=%.2f firstPixel=(%d,%d,%d%s)"),
            RawBytes.Num(),
            Channels,
            MinValue,
            MaxValue,
            Mean,
            RawBytes[0],
            RawBytes[1],
            RawBytes[2],
            Channels == 4 ? *FString::Printf(TEXT(",%d"), RawBytes[3]) : TEXT("")
        );
    }

    return true;
}

TArray<uint8> UMyActorComponent::ConvertRGBAtoRGB(const TArray<uint8>& RGBABytes) const
{
    const int32 PixelCount = RGBABytes.Num() / 4;
    TArray<uint8> RGBBytes;
    RGBBytes.SetNumUninitialized(PixelCount * 3);

    for (int32 i = 0; i < PixelCount; ++i)
    {
        const int32 Src = i * 4;
        const int32 Dst = i * 3;

        RGBBytes[Dst + 0] = RGBABytes[Src + 0];
        RGBBytes[Dst + 1] = RGBABytes[Src + 1];
        RGBBytes[Dst + 2] = RGBABytes[Src + 2];
    }

    return RGBBytes;
}

void UMyActorComponent::FlipImageRowsInPlace(TArray<uint8>& Bytes, int32 Width, int32 Height, int32 Channels) const
{
    const int32 RowStride = Width * Channels;
    TArray<uint8> TempRow;
    TempRow.SetNumUninitialized(RowStride);

    for (int32 Y = 0; Y < Height / 2; ++Y)
    {
        const int32 TopOffset = Y * RowStride;
        const int32 BottomOffset = (Height - 1 - Y) * RowStride;

        FMemory::Memcpy(TempRow.GetData(), Bytes.GetData() + TopOffset, RowStride);
        FMemory::Memcpy(Bytes.GetData() + TopOffset, Bytes.GetData() + BottomOffset, RowStride);
        FMemory::Memcpy(Bytes.GetData() + BottomOffset, TempRow.GetData(), RowStride);
    }
}
