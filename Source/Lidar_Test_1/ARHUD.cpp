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

    if (!DebugPanelClass)
    {
        static ConstructorHelpers::FClassFinder<UUserWidget> DebugPanelClassFinder(
            TEXT("/Game/Widget/WBP_DebugPanel")
        );
        if (DebugPanelClassFinder.Succeeded())
        {
            DebugPanelClass = DebugPanelClassFinder.Class;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AARHUD: cannot load widget class /Game/Widget/WBP_DebugPanel"));
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

    if (PoseDetectionComponent && CameraRenderTarget)
    {
        PoseDetectionComponent->SetRenderTarget(CameraRenderTarget);
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
            DebugPanelWidget->AddToViewport(1000);
            UE_LOG(LogTemp, Log, TEXT("AARHUD: DebugPanel created and added to viewport (Z=1000)"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AARHUD: DebugPanel create failed"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AARHUD: DebugPanel skipped (class or player controller missing)"));
    }

    UpdateDepthWidgetState();
    UpdateDebugPanelState();
    PushThoraxDepthToDebugPanel();

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

    if (DepthMaterial)
    {
        if (TObjectPtr<UARTexture> DepthTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap))
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

    if (CameraMaterial)
    {
        if (TObjectPtr<UARTexture> CameraTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage))
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

    if (!bLogThoraxDepthMean && !bShowThoraxDepthReadout && !bEnableThoraxDepthGraphUpdates)
    {
        return;
    }

    TimeSinceThoraxDepthLog += DeltaSeconds;
    TimeSinceThoraxDepthLog = 0.0f;

    if (!PoseDetectionComponent || !PoseDetectionComponent->BodyPoseManager)
    {
        bHasThoraxDepthReading = false;
        return;
    }

    const TArray<FPoseJoint>& Joints = PoseDetectionComponent->BodyPoseManager->DetectedJoints;
    FVector2D ThoraxUV = FVector2D::ZeroVector;
    if (!TryGetThoraxDepthUV(Joints, ThoraxUV))
    {
        bHasThoraxDepthReading = false;
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: no reliable thorax/chest joints"));
        return;
    }

    float MeanDepthValue = 0.0f;
    float MinDepthValue = 0.0f;
    float MaxDepthValue = 0.0f;
    int32 SampleCount = 0;
    if (!ComputeDepthMeanAtUV(ThoraxUV, MeanDepthValue, SampleCount, &MinDepthValue, &MaxDepthValue))
    {
        bHasThoraxDepthReading = false;
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: cannot sample depth texture"));
        return;
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

    if (!bLogThoraxDepthMean)
    {
        return;
    }

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
                ThoraxUV.X,
                ThoraxUV.Y,
                bDepthMaterialValuesAreNormalized ? TEXT("true") : TEXT("false"),
                DepthNearMeters,
                SafeFarMeters
            );
        }
        else
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("Thorax depth mean: %.3f m, min: %.3f m, max: %.3f m, samples=%d, uv=(%.3f, %.3f), normalized=%s, vizNear=%.2fm, vizFar=%.2fm"),
                MeanDepthMeters,
                MinDepthMeters,
                MaxDepthMeters,
                SampleCount,
                ThoraxUV.X,
                ThoraxUV.Y,
                bDepthMaterialValuesAreNormalized ? TEXT("true") : TEXT("false"),
                DepthNearMeters,
                SafeFarMeters
            );
        }
    }
    else
    {
        if (bLogThoraxDepthInMillimeters)
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("Thorax depth mean: %d mm (%.3f m), samples=%d, uv=(%.3f, %.3f), normalized=%s"),
                MeanDepthMillimeters,
                MeanDepthMeters,
                SampleCount,
                ThoraxUV.X,
                ThoraxUV.Y,
                bDepthMaterialValuesAreNormalized ? TEXT("true") : TEXT("false")
            );
        }
        else
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("Thorax depth mean: %.3f m, samples=%d, uv=(%.3f, %.3f), normalized=%s"),
                MeanDepthMeters,
                SampleCount,
                ThoraxUV.X,
                ThoraxUV.Y,
                bDepthMaterialValuesAreNormalized ? TEXT("true") : TEXT("false")
            );
        }
    }
}

void AARHUD::DrawHUD()
{
    Super::DrawHUD();

    DrawDepthToggleButton();
    DrawThoraxDepthReadout();

    if (!Canvas || !PoseDetectionComponent || !PoseDetectionComponent->BodyPoseManager)
    {
        return;
    }

    DrawJointsOverlay();

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

bool AARHUD::ComputeDepthMeanAtUV(
    const FVector2D& UV,
    float& OutMeanDepthValue,
    int32& OutSampleCount,
    float* OutMinDepthValue,
    float* OutMaxDepthValue
)
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

    const int32 CenterX = FMath::Clamp(FMath::RoundToInt(UV.X * static_cast<float>(Width - 1)), 0, Width - 1);
    const int32 CenterY = FMath::Clamp(FMath::RoundToInt(UV.Y * static_cast<float>(Height - 1)), 0, Height - 1);
    const int32 Radius = FMath::Max(1, ThoraxDepthSampleRadiusPixels);

    double DepthSum = 0.0;
    int32 Count = 0;
    float MinDepth = TNumericLimits<float>::Max();
    float MaxDepth = TNumericLimits<float>::Lowest();

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

            const FLinearColor& Pixel = PixelData[Y * Width + X];
            const float PixelDepth = Pixel.R;
            DepthSum += static_cast<double>(PixelDepth);
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

void AARHUD::DrawThoraxDepthReadout()
{
    if (!Canvas || !bShowThoraxDepthReadout)
    {
        return;
    }

    const FString ReadoutText = bHasThoraxDepthReading
        ? FString::Printf(TEXT("Thorax depth: %d mm"), LastThoraxDepthMillimeters)
        : TEXT("Thorax depth: -- mm");

    DrawText(
        ReadoutText,
        ThoraxDepthReadoutColor,
        ThoraxDepthReadoutPosition.X,
        ThoraxDepthReadoutPosition.Y,
        JointFont,
        ThoraxDepthReadoutScale,
        false
    );
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

    {
        static double LastGraphPushLogSeconds = 0.0;
        const double NowSeconds = FPlatformTime::Seconds();
        if (NowSeconds - LastGraphPushLogSeconds > 1.0)
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("AARHUD: PushThoraxDepthToDebugPanel -> history=%d current=%d hasDepth=%s targetFn=%s"),
                Params.DepthHistoryMillimeters.Num(),
                Params.CurrentDepthMillimeters,
                Params.bHasDepth ? TEXT("true") : TEXT("false"),
                *DebugPanelDepthUpdateFunctionName.ToString()
            );
            LastGraphPushLogSeconds = NowSeconds;
        }
    }

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
