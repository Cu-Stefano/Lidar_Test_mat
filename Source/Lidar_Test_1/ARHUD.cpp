// Fill out your copyright notice in the Description page of Project Settings.


#include "ARHUD.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "DefaultPoseDetector.h"
#include "BodyPoseManager.h"
#include "CameraFactorySingleton.h"
#include "PoseComponentFactorySingleton.h"
#include "IDepthCamera.h"

namespace
{
const FName DepthToggleHitBoxName(TEXT("DepthToggleHitBox"));

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
}

void AARHUD::BeginPlay()
{
    Super::BeginPlay();
    ValidateEditorAssignments();

    PoseComponentFactorySingleton& PoseFactory = PoseComponentFactorySingleton::GetInstance();
    PoseDetectorProvider = PoseFactory.CreatePoseComponent(TEXT("Default"), this);

    if (!PoseDetectorProvider.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("AARHUD: PoseDetectorProvider creation failed."));
    }

    CameraFactorySingleton& Factory = CameraFactorySingleton::GetInstance();
    DepthCameraProvider = Factory.CreateCamera(TEXT("Unreal"), this);

    if (!DepthCameraProvider.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("AARHUD: DepthCameraProvider creation failed."));
    }

    TObjectPtr<APlayerController> PC = GetOwningPlayerController();
    if (DepthWidgetClass && PC)
    {
        SceneDepthWidget = CreateWidget<UUserWidget>(PC, DepthWidgetClass);
        if (SceneDepthWidget)
        {
            SceneDepthWidget->AddToViewport();
        }
    }

    if (DepthMaterialBase)
    {
        DepthMaterial = UMaterialInstanceDynamic::Create(DepthMaterialBase, this);
    }

    if (IIPoseDetector* PD = PoseDetectorProvider.GetInterface(); PD && CameraRenderTarget)
    {
        PD->SetRenderTarget(CameraRenderTarget);
    }
    if (CameraMaterialBase)
    {
        CameraMaterial = UMaterialInstanceDynamic::Create(CameraMaterialBase, this);
    }

    if (DebugPanelClass && PC)
    {
        DebugPanelWidget = CreateWidget<UUserWidget>(PC, DebugPanelClass);
        if (DebugPanelWidget)
        {
            DebugPanelWidget->AddToViewport(10);
        }
    }

    UpdateDepthWidgetState();
    UpdateDebugPanelState();
    PushThoraxDepthToDebugPanel();

}

void AARHUD::ValidateEditorAssignments() const
{
    if (!DepthWidgetClass)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: DepthWidgetClass is null. Assign a widget (e.g. UI_DepthShower) in HUD class defaults.")
        );
    }

    if (!DepthMaterialBase)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: DepthMaterialBase is null. Assign MT_DepthMaterial in HUD class defaults.")
        );
    }

    if (!CameraMaterialBase)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: CameraMaterialBase is null. Assign MT_Camera in HUD class defaults.")
        );
    }

    if (!CameraRenderTarget)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: CameraRenderTarget is null. Assign RT_Camera in HUD class defaults.")
        );
    }

    if (!DebugPanelClass)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: DebugPanelClass is null. Depth graph panel updates will not be visible.")
        );
    }

    if (!ChestAreaMaterial)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: ChestAreaMaterial is null. Chest box will use fallback color only.")
        );
    }
}

void AARHUD::GetThoraxDepthHistory(TArray<int32>& OutHistoryMillimeters, int32& OutLatestDepthMillimeters, bool& bOutHasDepth) const
{
    OutHistoryMillimeters = ThoraxDepthHistoryMillimeters;
    OutLatestDepthMillimeters = LastThoraxDepthMillimeters;
    bOutHasDepth = bHasThoraxDepthReading;
}

void AARHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    IIDepthCamera* DepthCamera = DepthCameraProvider.GetInterface();

    if (DepthMaterial && DepthCamera)
    {
            if (UTexture* DepthTexture = DepthCamera->GetDepthTexture())
            {
                DepthMaterial->SetTextureParameterValue(DepthTextureParameterName, DepthTexture);
                DepthMaterial->SetScalarParameterValue(DepthNearMetersParameterName, DepthNearMeters);
                DepthMaterial->SetScalarParameterValue(DepthFarMetersParameterName, FMath::Max(DepthNearMeters + KINDA_SMALL_NUMBER, DepthFarMeters));
                if (bShowDepthOverlay)
                {
                    UpdateDepthWidgetState();
                }
            }
    }

        if (CameraMaterial && DepthCamera)
    {
            if (UTexture* CameraTexture = DepthCamera->GetCameraTexture())
            {
                CameraMaterial->SetTextureParameterValue(CameraTextureParameterName, CameraTexture);
            }
    }

    if (CameraRenderTarget && CameraMaterial)
    {
        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, CameraRenderTarget, CameraMaterial);
    }

    IIPoseDetector* PoseDetector = PoseDetectorProvider.GetInterface();

    if (PoseDetector && CameraRenderTarget)
    {
        PoseDetector->SetRenderTarget(CameraRenderTarget);
        PoseDetector->PerformPoseDetectionOnFrame();
    }

    if (!PoseDetector)
    {
        bHasThoraxDepthReading = false;
        bHasActiveThoraxBounds = false;
        bSmoothedBoundsInitialized = false;
        bSmoothedDepthInitialized = false;
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: PoseDetectorProvider not valid"));
        return;
    }

    const TArray<FPoseJoint>& Joints = PoseDetector->GetDetectedJoints();
    FVector2D ThoraxMinUV = FVector2D::ZeroVector;
    FVector2D ThoraxMaxUV = FVector2D::ZeroVector;
    if (!TryGetThoraxBoundsUV(Joints, ThoraxMinUV, ThoraxMaxUV))
    {
        bHasThoraxDepthReading = false;
        bHasActiveThoraxBounds = false;
        bSmoothedBoundsInitialized = false;
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: full thorax bounds not available (need both shoulders and hips)"));
        return;
    }

    if (bSmoothThoraxBoundsUV)
    {
        if (!bSmoothedBoundsInitialized)
        {
            SmoothedThoraxMinUV = ThoraxMinUV;
            SmoothedThoraxMaxUV = ThoraxMaxUV;
            bSmoothedBoundsInitialized = true;
        }
        else
        {
            const float BoundsAlpha = FMath::Clamp(ThoraxBoundsUVSmoothingAlpha, 0.01f, 1.0f);
            SmoothedThoraxMinUV = FMath::Lerp(SmoothedThoraxMinUV, ThoraxMinUV, BoundsAlpha);
            SmoothedThoraxMaxUV = FMath::Lerp(SmoothedThoraxMaxUV, ThoraxMaxUV, BoundsAlpha);
        }

        ThoraxMinUV = SmoothedThoraxMinUV;
        ThoraxMaxUV = SmoothedThoraxMaxUV;
    }

    ActiveThoraxMinUV = ThoraxMinUV;
    ActiveThoraxMaxUV = ThoraxMaxUV;
    bHasActiveThoraxBounds = true;

    const FVector2D ThoraxCenterUV = 0.5f * (ThoraxMinUV + ThoraxMaxUV);

    float MeanDepthValue = 0.0f;
    float MinDepthValue = 0.0f;
    float MaxDepthValue = 0.0f;
    float DepthSampleConfidence = 0.0f;
    int32 SampleCount = 0;
    if (!ComputeDepthMeanInBoundsUV(ThoraxMinUV, ThoraxMaxUV, MeanDepthValue, SampleCount, &MinDepthValue, &MaxDepthValue, &DepthSampleConfidence))
    {
        bHasThoraxDepthReading = false;
        bHasActiveThoraxBounds = false;
        bSmoothedDepthInitialized = false;
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Thorax depth mean skipped: cannot sample depth texture or confidence is too low (confidence=%.2f, min=%.2f)"),
            DepthSampleConfidence,
            MinDepthSampleConfidence
        );
        return;
    }

    if (bUseDepthSmoothing)
    {
        if (!bSmoothedDepthInitialized)
        {
            SmoothedThoraxDepthValue = MeanDepthValue;
            bSmoothedDepthInitialized = true;
        }
        else
        {
            const float DepthAlpha = FMath::Clamp(DepthSmoothingAlpha, 0.01f, 1.0f);
            SmoothedThoraxDepthValue = FMath::Lerp(SmoothedThoraxDepthValue, MeanDepthValue, DepthAlpha);
        }

        MeanDepthValue = SmoothedThoraxDepthValue;
    }

    const float SafeFarMeters = FMath::Max(DepthNearMeters + KINDA_SMALL_NUMBER, DepthFarMeters);
    const auto MaterialValueToMeters = [this, SafeFarMeters](const float DepthValue) -> float
    {
        if (bDepthMaterialValuesAreNormalized)
        {
            return FMath::Lerp(DepthNearMeters, SafeFarMeters, FMath::Clamp(DepthValue, 0.0f, 1.0f));
        }

        return FMath::Max(0.0f, DepthValue);
    };

    const float MeanDepthMeters = MaterialValueToMeters(MeanDepthValue);
    const float MinDepthMeters = MaterialValueToMeters(MinDepthValue);
    const float MaxDepthMeters = MaterialValueToMeters(MaxDepthValue);
    const int32 MeanDepthMillimeters = FMath::RoundToInt(MeanDepthMeters * 10000.0f);
    const int32 MinDepthMillimeters = FMath::RoundToInt(MinDepthMeters * 10000.0f);
    const int32 MaxDepthMillimeters = FMath::RoundToInt(MaxDepthMeters * 10000.0f);

    LastThoraxDepthMillimeters = MeanDepthMillimeters;
    bHasThoraxDepthReading = true;
    RecordThoraxDepthSample(MeanDepthMillimeters);
    PushThoraxDepthToDebugPanel();

    if (bLogThoraxDepthMinMax)
    {
        if (bLogThoraxDepthInMillimeters)
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("Thorax depth mean: %d mm (%.3f m), min: %d mm, max: %d mm, samples=%d, uv=(%.3f, %.3f), normalized=%s, vizNear=%.2fm, vizFar=%.2fm"),
                MeanDepthMillimeters,
                MeanDepthMeters,
                MinDepthMillimeters,
                MaxDepthMillimeters,
                SampleCount,
                ThoraxCenterUV.X,
                ThoraxCenterUV.Y,
                bDepthMaterialValuesAreNormalized ? TEXT("true") : TEXT("false"),
                DepthNearMeters,
                SafeFarMeters
            );

            UE_LOG(
                LogTemp,
                Verbose,
                TEXT("Thorax depth confidence: %.2f (threshold %.2f)"),
                DepthSampleConfidence,
                MinDepthSampleConfidence
            );
        }
        
    }
    
}

void AARHUD::DrawHUD()
{
    Super::DrawHUD();

    DrawDepthToggleButton();
    DrawChestSamplingArea();

    if (!Canvas || !PoseDetectorProvider.GetObject())
    {
        return;
    }

    if (bDrawJointDots || bDrawJointLabels)
    {
        DrawJointsOverlay();
    }

}

void AARHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);

    if (BoxName == DepthToggleHitBoxName)
    {
        bShowDepthOverlay = !bShowDepthOverlay;
        UpdateDepthWidgetState();
    }
}

void AARHUD::DrawJointsOverlay()
{
    if (!bDrawJointDots && !bDrawJointLabels)
    {
        return;
    }

    IIPoseDetector* PoseDetector = PoseDetectorProvider.GetInterface();
    if (!PoseDetector) return;

    const TArray<FPoseJoint>& Joints = PoseDetector->GetDetectedJoints();
    
    if (Joints.Num() == 0) return;

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

        if (bDrawJointDots && JointDotMaterial)
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
        else if (bDrawJointDots)
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

bool AARHUD::TryGetThoraxBoundsUV(const TArray<FPoseJoint>& Joints, FVector2D& OutMinUV, FVector2D& OutMaxUV) const
{
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

    if (!LeftShoulder || !RightShoulder || !LeftHip || !RightHip)
    {
        return false;
    }

    TArray<FVector2D> BoundsPoints;
    BoundsPoints.Reserve(4);
    BoundsPoints.Add(FVector2D(LeftShoulder->X, LeftShoulder->Y));
    BoundsPoints.Add(FVector2D(RightShoulder->X, RightShoulder->Y));
    BoundsPoints.Add(FVector2D(LeftHip->X, LeftHip->Y));
    BoundsPoints.Add(FVector2D(RightHip->X, RightHip->Y));

    bool bAllNormalized = true;
    for (const FVector2D& Point : BoundsPoints)
    {
        if (Point.X < 0.0f || Point.X > 1.0f || Point.Y < 0.0f || Point.Y > 1.0f)
        {
            bAllNormalized = false;
            break;
        }
    }

    if (!bAllNormalized && CameraRenderTarget && CameraRenderTarget->SizeX > 0 && CameraRenderTarget->SizeY > 0)
    {
        const float InvWidth = 1.0f / static_cast<float>(CameraRenderTarget->SizeX);
        const float InvHeight = 1.0f / static_cast<float>(CameraRenderTarget->SizeY);
        for (FVector2D& Point : BoundsPoints)
        {
            Point.X *= InvWidth;
            Point.Y *= InvHeight;
        }
    }

    for (FVector2D& Point : BoundsPoints)
    {
        Point.X = FMath::Clamp(Point.X, 0.0f, 1.0f);
        Point.Y = FMath::Clamp(Point.Y, 0.0f, 1.0f);
        if (bFlipNormalizedJointY)
        {
            Point.Y = 1.0f - Point.Y;
        }
    }

    float MinX = TNumericLimits<float>::Max();
    float MinY = TNumericLimits<float>::Max();
    float MaxX = TNumericLimits<float>::Lowest();
    float MaxY = TNumericLimits<float>::Lowest();
    for (const FVector2D& Point : BoundsPoints)
    {
        MinX = FMath::Min(MinX, Point.X);
        MinY = FMath::Min(MinY, Point.Y);
        MaxX = FMath::Max(MaxX, Point.X);
        MaxY = FMath::Max(MaxY, Point.Y);
    }

    OutMinUV = FVector2D(MinX, MinY);
    OutMaxUV = FVector2D(MaxX, MaxY);
    return true;
}

bool AARHUD::ComputeDepthMeanInBoundsUV(
    const FVector2D& MinUV,
    const FVector2D& MaxUV,
    float& OutMeanDepthValue,
    int32& OutSampleCount,
    float* OutMinDepthValue,
    float* OutMaxDepthValue,
    float* OutDepthSampleConfidence)
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
        ETextureRenderTargetFormat DepthRTFormat;
        
        DepthRTFormat = bUseFloat32DepthSampling
            ? ETextureRenderTargetFormat::RTF_RGBA32f
            : ETextureRenderTargetFormat::RTF_RGBA16f;
        
        DepthDebugRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(
            this,
            DepthRTWidth,
            DepthRTHeight,
            DepthRTFormat
        );

        if (DepthDebugRenderTarget)
        {
            DepthDebugRenderTarget->bForceLinearGamma = true;
            DepthDebugRenderTarget->UpdateResourceImmediate(false);
        }
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

    TArray<FLinearColor> PixelData;
    FReadSurfaceDataFlags ReadFlags;
    ReadFlags.SetLinearToGamma(false);
    if (!RTResource->ReadLinearColorPixels(PixelData, ReadFlags) || PixelData.Num() == 0)
    {
        return false;
    }

    const int32 Width = DepthDebugRenderTarget->SizeX;
    const int32 Height = DepthDebugRenderTarget->SizeY;

    const int32 StartX = FMath::Clamp(FMath::RoundToInt(FMath::Min(MinUV.X, MaxUV.X) * static_cast<float>(Width - 1)), 0, Width - 1);
    const int32 EndX = FMath::Clamp(FMath::RoundToInt(FMath::Max(MinUV.X, MaxUV.X) * static_cast<float>(Width - 1)), 0, Width - 1);
    const int32 StartY = FMath::Clamp(FMath::RoundToInt(FMath::Min(MinUV.Y, MaxUV.Y) * static_cast<float>(Height - 1)), 0, Height - 1);
    const int32 EndY = FMath::Clamp(FMath::RoundToInt(FMath::Max(MinUV.Y, MaxUV.Y) * static_cast<float>(Height - 1)), 0, Height - 1);

    const float SafeMaxValidDepthMeters = FMath::Max(MinValidDepthMeters + KINDA_SMALL_NUMBER, MaxValidDepthMeters);
    const float SafeMaxStdDev = FMath::Max(0.001f, MaxDepthStdDevForConfidenceMeters);

    double DepthSum = 0.0;
    double DepthSquaredSum = 0.0;
    int32 CandidateCount = 0;
    int32 Count = 0;
    float MinDepth = TNumericLimits<float>::Max();
    float MaxDepth = TNumericLimits<float>::Lowest();

    for (int32 Y = StartY; Y <= EndY; ++Y)
    {
        for (int32 X = StartX; X <= EndX; ++X)
        {
            ++CandidateCount;

            const FLinearColor& Pixel = PixelData[Y * Width + X];
            const float PixelDepth = Pixel.R;

            if (!FMath::IsFinite(PixelDepth) || PixelDepth < MinValidDepthMeters || PixelDepth > SafeMaxValidDepthMeters)
            {
                continue;
            }

            DepthSum += static_cast<double>(PixelDepth);
            DepthSquaredSum += static_cast<double>(PixelDepth) * static_cast<double>(PixelDepth);
            MinDepth = FMath::Min(MinDepth, PixelDepth);
            MaxDepth = FMath::Max(MaxDepth, PixelDepth);
            ++Count;
        }
    }

    if (Count <= 0)
    {
        return false;
    }

    OutSampleCount = Count;
    OutMeanDepthValue = static_cast<float>(DepthSum / static_cast<double>(Count));

    float DepthSampleConfidence = 1.0f;
    if (CandidateCount > 0)
    {
        const float ValidRatio = static_cast<float>(Count) / static_cast<float>(CandidateCount);
        const double Mean = DepthSum / static_cast<double>(Count);
        const double MeanSquared = Mean * Mean;
        const double Variance = FMath::Max(0.0, (DepthSquaredSum / static_cast<double>(Count)) - MeanSquared);
        const float StdDev = static_cast<float>(FMath::Sqrt(Variance));
        const float Stability = 1.0f - FMath::Clamp(StdDev / SafeMaxStdDev, 0.0f, 1.0f);
        DepthSampleConfidence = ValidRatio * Stability;
    }

    if (OutDepthSampleConfidence)
    {
        *OutDepthSampleConfidence = DepthSampleConfidence;
    }

    if (bUseDepthSampleConfidenceFilter && DepthSampleConfidence < MinDepthSampleConfidence)
    {
        return false;
    }

    if (OutMinDepthValue)
    {
        *OutMinDepthValue = MinDepth;
    }
    if (OutMaxDepthValue)
    {
        *OutMaxDepthValue = MaxDepth;
    }

    return true;
}

void AARHUD::PushMaterialToWidget(UMaterialInterface* Material)
{
    if (!SceneDepthWidget || !Material)
    {
        return;
    }

    TObjectPtr<UFunction> SetImageMaterialFunction = SceneDepthWidget->FindFunction(TEXT("SetImageMaterial"));
    if (!SetImageMaterialFunction)
    {
        return;
    }

    struct FSetImageMaterialParams
    {
        UMaterialInterface* Material;
    };

    FSetImageMaterialParams Params;
    Params.Material = Material;
    SceneDepthWidget->ProcessEvent(SetImageMaterialFunction, &Params);
}

void AARHUD::UpdateDepthWidgetState()
{
    if (!SceneDepthWidget)
    {
        return;
    }

    if (!bShowDepthOverlay)
    {
        SceneDepthWidget->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    if (DepthMaterial)
    {
        PushMaterialToWidget(DepthMaterial);
    }

    SceneDepthWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void AARHUD::UpdateDebugPanelState()
{
    if (!DebugPanelWidget)
    {
        return;
    }

    DebugPanelWidget->SetVisibility(
        bShowDebugPanel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void AARHUD::DrawDepthToggleButton()
{
    if (!Canvas || !bShowDepthToggleButton)
    {
        return;
    }

    const FLinearColor FillColor = bShowDepthOverlay ? DepthToggleButtonOnColor : DepthToggleButtonOffColor;
    DrawRect(
        FillColor,
        DepthToggleButtonPosition.X,
        DepthToggleButtonPosition.Y,
        DepthToggleButtonSize.X,
        DepthToggleButtonSize.Y
    );

    const FString Label = bShowDepthOverlay ? TEXT("Depth: ON") : TEXT("Depth: OFF");
    DrawText(
        Label,
        FLinearColor::White,
        DepthToggleButtonPosition.X + 12.0f,
        DepthToggleButtonPosition.Y + 12.0f,
        JointFont,
        0.9f,
        false
    );

    AddHitBox(
        DepthToggleButtonPosition,
        DepthToggleButtonSize,
        DepthToggleHitBoxName,
        true,
        0
    );
}

void AARHUD::DrawChestSamplingArea()
{
    if (!Canvas || !bShowChestSamplingArea)
    {
        return;
    }

    if (!bHasActiveThoraxBounds)
    {
        return;
    }

    const float Width = Canvas->ClipX;
    const float Height = Canvas->ClipY;
    if (Width <= 1.0f || Height <= 1.0f)
    {
        return;
    }

    const float MinX = FMath::Min(ActiveThoraxMinUV.X, ActiveThoraxMaxUV.X);
    const float MaxX = FMath::Max(ActiveThoraxMinUV.X, ActiveThoraxMaxUV.X);
    const float MinY = FMath::Min(ActiveThoraxMinUV.Y, ActiveThoraxMaxUV.Y);
    const float MaxY = FMath::Max(ActiveThoraxMinUV.Y, ActiveThoraxMaxUV.Y);

    float RectX = MinX * Width;
    float RectY = MinY * Height;
    float RectW = (MaxX - MinX) * Width;
    float RectH = (MaxY - MinY) * Height;

    RectX = FMath::Clamp(RectX, 0.0f, Width - 1.0f);
    RectY = FMath::Clamp(RectY, 0.0f, Height - 1.0f);
    RectW = FMath::Clamp(RectW, 1.0f, Width - RectX);
    RectH = FMath::Clamp(RectH, 1.0f, Height - RectY);

    if (ChestAreaMaterial)
    {
        DrawMaterial(
            ChestAreaMaterial,
            RectX,
            RectY,
            RectW,
            RectH,
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
        DrawRect(ChestAreaFallbackColor, RectX, RectY, RectW, RectH);
    }

    if (ChestAreaOutlineThickness > 0.0f)
    {
        DrawLine(RectX, RectY, RectX + RectW, RectY, ChestAreaOutlineColor, ChestAreaOutlineThickness);
        DrawLine(RectX, RectY + RectH, RectX + RectW, RectY + RectH, ChestAreaOutlineColor, ChestAreaOutlineThickness);
        DrawLine(RectX, RectY, RectX, RectY + RectH, ChestAreaOutlineColor, ChestAreaOutlineThickness);
        DrawLine(RectX + RectW, RectY, RectX + RectW, RectY + RectH, ChestAreaOutlineColor, ChestAreaOutlineThickness);
    }
}

void AARHUD::RecordThoraxDepthSample(const int32 DepthMillimeters)
{
    ThoraxDepthHistoryMillimeters.Add(DepthMillimeters);

    const int32 MaxSamples = FMath::Max(1, ThoraxDepthHistoryMaxSamples);
    if (ThoraxDepthHistoryMillimeters.Num() > MaxSamples)
    {
        const int32 SamplesToTrim = ThoraxDepthHistoryMillimeters.Num() - MaxSamples;
        ThoraxDepthHistoryMillimeters.RemoveAt(0, SamplesToTrim, EAllowShrinking::No);
    }
}

void AARHUD::PushThoraxDepthToDebugPanel()
{
    if (!DebugPanelWidget || DebugPanelDepthUpdateFunctionName.IsNone())
    {
        return;
    }

    UFunction* UpdateFunction = DebugPanelWidget->FindFunction(DebugPanelDepthUpdateFunctionName);
    if (!UpdateFunction)
    {
        if (!bLoggedMissingDepthGraphFunction)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("AARHUD: Debug panel function '%s' not found. Add it in WBP_DebugPanel to receive depth graph updates."),
                *DebugPanelDepthUpdateFunctionName.ToString()
            );
            bLoggedMissingDepthGraphFunction = true;
        }
        return;
    }

    bLoggedMissingDepthGraphFunction = false;

    struct FUpdateThoraxDepthGraphParams
    {
        TArray<int32> DepthHistoryMillimeters;
        int32 CurrentDepthMillimeters = 0;
        bool bHasDepth = false;
    };

    FUpdateThoraxDepthGraphParams Params;
    Params.DepthHistoryMillimeters = ThoraxDepthHistoryMillimeters;
    Params.CurrentDepthMillimeters = LastThoraxDepthMillimeters;
    Params.bHasDepth = bHasThoraxDepthReading;

    DebugPanelWidget->ProcessEvent(UpdateFunction, &Params);
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
