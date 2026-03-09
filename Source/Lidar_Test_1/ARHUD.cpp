// Fill out your copyright notice in the Description page of Project Settings.


#include "ARHUD.h"

#include "ARBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "PoseDetectionComponent.h"
#include "BodyPoseManager.h"

namespace
{
enum class EThoraxJointRole : uint8
{
    Unknown,
    Torso,
    LeftShoulder,
    RightShoulder,
    LeftHip,
    RightHip,
};

const TMap<FString, EThoraxJointRole>& GetThoraxJointRoleDictionary()
{
    static const TMap<FString, EThoraxJointRole> Dictionary = {
        // Vision joint names seen in logs.
        {TEXT("neck_1_joint"), EThoraxJointRole::Torso},
        {TEXT("root"), EThoraxJointRole::Torso},
        {TEXT("left_shoulder_1_joint"), EThoraxJointRole::LeftShoulder},
        {TEXT("right_shoulder_1_joint"), EThoraxJointRole::RightShoulder},
        {TEXT("left_upleg_joint"), EThoraxJointRole::LeftHip},
        {TEXT("right_upleg_joint"), EThoraxJointRole::RightHip},

        // Compatibility aliases.
        {TEXT("left_shoulder"), EThoraxJointRole::LeftShoulder},
        {TEXT("right_shoulder"), EThoraxJointRole::RightShoulder},
        {TEXT("left_hip"), EThoraxJointRole::LeftHip},
        {TEXT("right_hip"), EThoraxJointRole::RightHip},
        {TEXT("thorax"), EThoraxJointRole::Torso},
        {TEXT("chest"), EThoraxJointRole::Torso},
        {TEXT("sternum"), EThoraxJointRole::Torso},
        {TEXT("torso"), EThoraxJointRole::Torso},
        {TEXT("spine_chest"), EThoraxJointRole::Torso},
        {TEXT("mid_shoulder"), EThoraxJointRole::Torso},
    };

    return Dictionary;
}

EThoraxJointRole ResolveThoraxJointRole(const FString& RawName)
{
    const FString NameLower = RawName.ToLower();
    if (const EThoraxJointRole* Role = GetThoraxJointRoleDictionary().Find(NameLower))
    {
        return *Role;
    }

    return EThoraxJointRole::Unknown;
}
}

AARHUD::AARHUD()
{
    PrimaryActorTick.bCanEverTick = true;
    PoseDetectionComponent = CreateDefaultSubobject<UPoseDetectionComponent>(TEXT("PoseDetectionComponent"));

    // Pure C++ setup: auto-assign project assets so BP defaults are optional.
    if (!DepthWidgetClass)
    {
        static ConstructorHelpers::FClassFinder<UUserWidget> DepthWidgetClassFinder(
            TEXT("/Game/Widget/UI_DepthShower")
        );
        if (DepthWidgetClassFinder.Succeeded())
        {
            DepthWidgetClass = DepthWidgetClassFinder.Class;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AARHUD: cannot load widget class /Game/Widget/UI_DepthShower"));
        }
    }

    if (!DepthMaterialBase)
    {
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> DepthMaterialFinder(
            TEXT("/Game/Materials/MT_DepthMaterial.MT_DepthMaterial")
        );
        if (DepthMaterialFinder.Succeeded())
        {
            DepthMaterialBase = DepthMaterialFinder.Object;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AARHUD: cannot load material /Game/Materials/MT_DepthMaterial"));
        }
    }

    if (!CameraMaterialBase)
    {
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> CameraMaterialFinder(
            TEXT("/Game/Materials/MT_Camera.MT_Camera")
        );
        if (CameraMaterialFinder.Succeeded())
        {
            CameraMaterialBase = CameraMaterialFinder.Object;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AARHUD: cannot load material /Game/Materials/MT_Camera"));
        }
    }

    if (!CameraRenderTarget)
    {
        static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> CameraRenderTargetFinder(
            TEXT("/Game/Materials/RT_Camera.RT_Camera")
        );
        if (CameraRenderTargetFinder.Succeeded())
        {
            CameraRenderTarget = CameraRenderTargetFinder.Object;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AARHUD: cannot load render target /Game/Materials/RT_Camera"));
        }
    }

    if (!JointFont)
    {
        static ConstructorHelpers::FObjectFinder<UFont> RobotoFontFinder(
            TEXT("/Engine/EngineFonts/Roboto.Roboto")
        );
        if (RobotoFontFinder.Succeeded())
        {
            JointFont = RobotoFontFinder.Object;
        }
    }
}

void AARHUD::BeginPlay()
{
    Super::BeginPlay();

    // Depth widget intentionally disabled: keep depth processing without showing UI overlay.
    SceneDepthWidget = nullptr;

    if (DepthMaterialBase)
    {
        DepthMaterial = UMaterialInstanceDynamic::Create(DepthMaterialBase, this);
    }

    if (PoseDetectionComponent && CameraRenderTarget)
    {
        PoseDetectionComponent->SetRenderTarget(CameraRenderTarget);
    }

    if (CameraMaterialBase)
    {
        CameraMaterial = UMaterialInstanceDynamic::Create(CameraMaterialBase, this);
    }

}

void AARHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (DepthMaterial)
    {
        if (UARTexture* DepthTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap))
        {
            DepthMaterial->SetTextureParameterValue(DepthTextureParameterName, DepthTexture);
        }
    }

    if (CameraMaterial)
    {
        if (UARTexture* CameraTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage))
        {
            CameraMaterial->SetTextureParameterValue(CameraTextureParameterName, CameraTexture);
        }
    }

    if (CameraRenderTarget && CameraMaterial)
    {
        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, CameraRenderTarget, CameraMaterial);
    }

    if (PoseDetectionComponent && CameraRenderTarget)
    {
        PoseDetectionComponent->SetRenderTarget(CameraRenderTarget);
        PoseDetectionComponent->PerformPoseDetectionOnFrame();
    }

    if (!bLogThoraxDepthMean)
    {
        return;
    }

    TimeSinceThoraxDepthLog += DeltaSeconds;
    if (TimeSinceThoraxDepthLog < ThoraxDepthLogIntervalSeconds)
    {
        return;
    }
    TimeSinceThoraxDepthLog = 0.0f;

    if (!PoseDetectionComponent || !PoseDetectionComponent->BodyPoseManager)
    {
        return;
    }

    const TArray<FPoseJoint>& Joints = PoseDetectionComponent->BodyPoseManager->DetectedJoints;
    FVector2D ThoraxUV = FVector2D::ZeroVector;
    if (!TryGetThoraxDepthUV(Joints, ThoraxUV))
    {
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: no reliable thorax/chest joints"));
        return;
    }

    float MeanDepthRaw = 0.0f;
    int32 SampleCount = 0;
    if (!ComputeDepthMeanAtUV(ThoraxUV, MeanDepthRaw, SampleCount))
    {
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: cannot sample depth texture"));
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Thorax depth mean: %.2f (raw 0..255), samples=%d, uv=(%.3f, %.3f)"),
        MeanDepthRaw,
        SampleCount,
        ThoraxUV.X,
        ThoraxUV.Y
    );
}

void AARHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !PoseDetectionComponent || !PoseDetectionComponent->BodyPoseManager)
    {
        return;
    }

    DrawJointsOverlay();

}

void AARHUD::DrawJointsOverlay()
{
    if (!PoseDetectionComponent)        return;
    if (!PoseDetectionComponent->BodyPoseManager) return;
    
    const TArray<FPoseJoint>& Joints = PoseDetectionComponent->BodyPoseManager->DetectedJoints;
    
    if (Joints.Num() == 0) return;  // array vuoto, niente da disegnare

    for (const FPoseJoint& Joint : Joints)
    {
        if (Joint.Confidence < MinJointConfidence)
        {
            continue;
        }

        const FVector2D ScreenPos = ToScreenSpace(Joint.X, Joint.Y);
        const float DotHalf = JointDotSize * 0.5f;
        const float DotX = ScreenPos.X - DotHalf;
        const float DotY = ScreenPos.Y - DotHalf;

        if (JointDotMaterial)
        {
            DrawMaterial(
                JointDotMaterial,
            DotX,
            DotY,
                JointDotSize,
                JointDotSize,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                1.0f,
                false,
                0.0f,
                FVector2D::ZeroVector
            );
        }
        else
        {
            // Fallback marker when no dot material is assigned in the HUD defaults.
            DrawRect(JointTextColor, DotX, DotY, JointDotSize, JointDotSize);
        }

        if (bDrawJointLabels)
        {
            const FString JointLabel = Joint.Name.IsEmpty() ? TEXT("joint") : Joint.Name;
            DrawText(
                JointLabel,
                JointTextColor,
                ScreenPos.X,
                ScreenPos.Y + JointTextYOffset,
                JointFont,
                JointTextScale,
                false
            );
        }
    }
}

bool AARHUD::TryGetThoraxDepthUV(const TArray<FPoseJoint>& Joints, FVector2D& OutUV) const
{
    const FPoseJoint* BestTorsoJoint = nullptr;
    float BestTorsoConfidence = -1.0f;

    const FPoseJoint* LeftShoulder = nullptr;
    const FPoseJoint* RightShoulder = nullptr;
    const FPoseJoint* LeftHip = nullptr;
    const FPoseJoint* RightHip = nullptr;

    for (const FPoseJoint& Joint : Joints)
    {
        if (Joint.Confidence < MinJointConfidence)
        {
            continue;
        }

        const EThoraxJointRole JointRole = ResolveThoraxJointRole(Joint.Name);

        if (JointRole == EThoraxJointRole::Torso)
        {
            if (Joint.Confidence > BestTorsoConfidence)
            {
                BestTorsoJoint = &Joint;
                BestTorsoConfidence = Joint.Confidence;
            }
        }

        if (JointRole == EThoraxJointRole::LeftShoulder)
        {
            LeftShoulder = &Joint;
        }
        else if (JointRole == EThoraxJointRole::RightShoulder)
        {
            RightShoulder = &Joint;
        }
        else if (JointRole == EThoraxJointRole::LeftHip)
        {
            LeftHip = &Joint;
        }
        else if (JointRole == EThoraxJointRole::RightHip)
        {
            RightHip = &Joint;
        }
    }
    if (BestTorsoJoint)
    {
        OutUV = FVector2D(BestTorsoJoint->X, BestTorsoJoint->Y);
    }
    else
    {
        if (!LeftShoulder || !RightShoulder)
        {
            return false;
        }

        const FVector2D ShoulderCenter = FVector2D(
            0.5f * (LeftShoulder->X + RightShoulder->X),
            0.5f * (LeftShoulder->Y + RightShoulder->Y)
        );

        if (LeftHip && RightHip)
        {
            const FVector2D HipCenter = FVector2D(
                0.5f * (LeftHip->X + RightHip->X),
                0.5f * (LeftHip->Y + RightHip->Y)
            );

            OutUV = ShoulderCenter * 0.65f + HipCenter * 0.35f;
        }
        else
        {
            OutUV = ShoulderCenter;
        }
    }

    const bool bNormalizedX = OutUV.X >= 0.0f && OutUV.X <= 1.0f;
    const bool bNormalizedY = OutUV.Y >= 0.0f && OutUV.Y <= 1.0f;

    if ((!bNormalizedX || !bNormalizedY) && CameraRenderTarget && CameraRenderTarget->SizeX > 0 && CameraRenderTarget->SizeY > 0)
    {
        OutUV.X /= static_cast<float>(CameraRenderTarget->SizeX);
        OutUV.Y /= static_cast<float>(CameraRenderTarget->SizeY);
    }

    OutUV.X = FMath::Clamp(OutUV.X, 0.0f, 1.0f);
    OutUV.Y = FMath::Clamp(OutUV.Y, 0.0f, 1.0f);

    if (bFlipNormalizedJointY)
    {
        OutUV.Y = 1.0f - OutUV.Y;
    }

    return true;
}

bool AARHUD::ComputeDepthMeanAtUV(const FVector2D& UV, float& OutMeanDepthRaw, int32& OutSampleCount)
{
    if (!DepthMaterial)
    {
        return false;
    }

    int32 DepthRTWidth = 256;
    int32 DepthRTHeight = 256;

    if (CameraRenderTarget && CameraRenderTarget->SizeX > 0 && CameraRenderTarget->SizeY > 0)
    {
        DepthRTWidth = CameraRenderTarget->SizeX;
        DepthRTHeight = CameraRenderTarget->SizeY;
    }

    if (!DepthDebugRenderTarget || DepthDebugRenderTarget->SizeX != DepthRTWidth || DepthDebugRenderTarget->SizeY != DepthRTHeight)
    {
        DepthDebugRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
            this,
            DepthRTWidth,
            DepthRTHeight,
            ETextureRenderTargetFormat::RTF_RGBA8
        );
    }

    if (!DepthDebugRenderTarget)
    {
        return false;
    }

    UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, DepthDebugRenderTarget, DepthMaterial);

    FTextureRenderTargetResource* RTResource = DepthDebugRenderTarget->GameThread_GetRenderTargetResource();
    if (!RTResource)
    {
        return false;
    }

    TArray<FColor> PixelData;
    if (!RTResource->ReadPixels(PixelData) || PixelData.Num() == 0)
    {
        return false;
    }

    const int32 Width = DepthDebugRenderTarget->SizeX;
    const int32 Height = DepthDebugRenderTarget->SizeY;

    const int32 CenterX = FMath::Clamp(FMath::RoundToInt(UV.X * static_cast<float>(Width - 1)), 0, Width - 1);
    const int32 CenterY = FMath::Clamp(FMath::RoundToInt(UV.Y * static_cast<float>(Height - 1)), 0, Height - 1);
    const int32 Radius = FMath::Max(1, ThoraxDepthSampleRadiusPixels);

    double DepthSum = 0.0;
    int32 Count = 0;

    for (int32 Y = FMath::Max(0, CenterY - Radius); Y <= FMath::Min(Height - 1, CenterY + Radius); ++Y)
    {
        for (int32 X = FMath::Max(0, CenterX - Radius); X <= FMath::Min(Width - 1, CenterX + Radius); ++X)
        {
            const int32 DX = X - CenterX;
            const int32 DY = Y - CenterY;
            if (DX * DX + DY * DY > Radius * Radius)
            {
                continue;
            }

            const FColor& Pixel = PixelData[Y * Width + X];
            const float PixelDepthRaw = static_cast<float>(Pixel.R);

            DepthSum += PixelDepthRaw;
            ++Count;
        }
    }

    if (Count <= 0)
    {
        return false;
    }

    OutSampleCount = Count;
    OutMeanDepthRaw = static_cast<float>(DepthSum / static_cast<double>(Count));

    return true;
}

void AARHUD::PushDepthMaterialToWidget()
{
    if (!SceneDepthWidget || !DepthMaterial)
    {
        return;
    }

    UFunction* SetImageMaterialFunction = SceneDepthWidget->FindFunction(TEXT("SetImageMaterial"));
    if (!SetImageMaterialFunction)
    {
        return;
    }

    struct FSetImageMaterialParams
    {
        UMaterialInterface* Material;
    };

    FSetImageMaterialParams Params;
    Params.Material = DepthMaterial;
    SceneDepthWidget->ProcessEvent(SetImageMaterialFunction, &Params);
}

FVector2D AARHUD::ToScreenSpace(float X, float Y) const
{
    const float Width = Canvas ? Canvas->ClipX : 0.0f;
    const float Height = Canvas ? Canvas->ClipY : 0.0f;

    const bool bNormalizedX = (X >= 0.0f && X <= 1.0f);
    const bool bNormalizedY = (Y >= 0.0f && Y <= 1.0f);

    const float ScreenX = bNormalizedX ? X * Width : X;
    float ScreenY = bNormalizedY ? Y * Height : Y;

    if (bNormalizedY && bFlipNormalizedJointY)
    {
        ScreenY = Height - ScreenY;
    }

    return FVector2D(ScreenX, ScreenY);
}
