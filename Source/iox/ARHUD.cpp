// Fill out your copyright notice in the Description page of Project Settings.


#include "ARHUD.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "DefaultPoseDetector.h"
#include "BodyPoseManager.h"
#include "CameraFactorySingleton.h"
#include "PoseComponentFactorySingleton.h"
#include "ICameraWithDepth.h"
#include "UDepthGraphWidget.h"
#include "MainPanel.h"
#include "Constants.h"
// Note: Enums, joint dictionaries and helper functions moved to ThoraxJointHelper.h/.cpp

AARHUD::AARHUD()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AARHUD::BeginPlay()
{
    Super::BeginPlay();
    ValidateEditorAssignments();

    // Pose Component 
    PoseComponentFactorySingleton& PoseFactory = PoseComponentFactorySingleton::GetInstance();
    PoseDetectorProvider = PoseFactory.CreatePoseComponent(TEXT("Default"), this);

    if (!PoseDetectorProvider.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("AARHUD: PoseDetectorProvider creation failed."));
    }

    // Camera + Depth Component
    CameraFactorySingleton& Factory = CameraFactorySingleton::GetInstance();
    CameraWithDepthProvider = Factory.CreateCamera(TEXT("Unreal"), this);

    if (!CameraWithDepthProvider.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("AARHUD: CameraWithDepthProvider creation failed."));
    }

    TObjectPtr<APlayerController> PC = GetOwningPlayerController();
    if (DepthWidgetClass && PC)
    {
        SceneDepthWidget = CreateWidget<UUserWidget>(PC, DepthWidgetClass);
        if (SceneDepthWidget)
        {
            SceneDepthWidget->AddToViewport(-1);
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

    if (MainPanelClass && PC)
    {
        MainPanelWidget = CreateWidget<UMainPanel>(PC, MainPanelClass);
        if (MainPanelWidget)
        {
            MainPanelWidget->AddToViewport(5);
        }
    }

    UpdateDepthWidgetState();
    UpdateMainPanelState();
    UpdateMainPanelDepth();

    DepthSampler = NewObject<UDepthSampler>(this);
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

    if (!DepthRenderTarget)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: DepthRenderTarget is null. Assign RT_Depth in HUD class defaults.")
        );
    }

    if (!MainPanelClass)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("AARHUD: MainPanelClass is null. Depth graph panel updates will not be visible.")
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

void AARHUD::GetThoraxDepthHistory(TArray<float>& OutHistory, float& OutLatestDepth, bool& bOutHasDepth) const
{
    OutHistory = ThoraxDepthHistory;
    OutLatestDepth = LastThoraxDepth;
    bOutHasDepth = bHasThoraxDepthReading;
}

void AARHUD::GetSternumDepthHistory(TArray<float>& OutHistory, float& OutLatestDepth, bool& bOutHasDepth) const
{
    OutHistory = SternumDepthHistory;
    OutLatestDepth = LastSternumDepth;
    bOutHasDepth = bHasSternumDepthReading;
}

void AARHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ICameraWithDepth* CameraWithDepth = CameraWithDepthProvider.GetInterface();

    if (DepthMaterial && CameraWithDepth)
    {
            if (UTexture* DepthTexture = CameraWithDepth->GetDepthTexture())
            {
                DepthMaterial->SetTextureParameterValue(DepthTextureParameterName, DepthTexture);
                if (bShowDepthOverlay)
                {
                    UpdateDepthWidgetState();
                }
            }
    }

        if (CameraMaterial && CameraWithDepth)
    {
            if (UTexture* CameraTexture = CameraWithDepth->GetCameraTexture())
            {
                CameraMaterial->SetTextureParameterValue(CameraTextureParameterName, CameraTexture);
            }
    }

    if (CameraRenderTarget && CameraMaterial)
    {
        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, CameraRenderTarget, CameraMaterial);
    }

    if (DepthRenderTarget && DepthMaterial)
    {
        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, DepthRenderTarget, DepthMaterial);
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
        bHasActiveSternumBounds = false;
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
        bHasActiveSternumBounds = false;
        UE_LOG(LogTemp, Warning, TEXT("Thorax depth mean skipped: full thorax bounds not available (need both shoulders and hips)"));
        return;
    }

    ActiveThoraxMinUV = ThoraxMinUV;
    ActiveThoraxMaxUV = ThoraxMaxUV;

    bHasActiveSternumBounds = TryGetSternumBoundsUV(ActiveThoraxMinUV, ActiveThoraxMaxUV, ActiveSternumMinUV, ActiveSternumMaxUV, SternumAreaSize);

    bHasActiveThoraxBounds = true;

    if (!DepthSampler || !DepthSampler->CaptureFrame(DepthMaterial, CameraRenderTarget, bUseFloat32DepthSampling))
    {
        bHasThoraxDepthReading = false;
        bHasSternumDepthReading = false;
        UE_LOG(LogTemp, Warning, TEXT("Depth frame data capture failed"));
        return;
    }

    const FVector2D ThoraxCenterUV = 0.5f * (ThoraxMinUV + ThoraxMaxUV);

    float MeanDepthValue = 0.0f;
    float MinDepthValue = 0.0f;
    float MaxDepthValue = 0.0f;
    float DepthSampleConfidence = 0.0f;
    int32 SampleCount = 0;

    //Thoarx
    if (!DepthSampler->ComputeMeanInBoundsUV(ThoraxMinUV, ThoraxMaxUV, MeanDepthValue, SampleCount, MinDepthValue, MaxDepthValue, DepthSampleConfidence))
    {
        bHasThoraxDepthReading = false;
        bHasActiveThoraxBounds = false;
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Thorax depth mean skipped: cannot sample depth texture or confidence is too low (confidence=%.2f, min=%.2f)"),
            DepthSampleConfidence,
            MinDepthSampleConfidence
        );
        return;
    }

    const float MeanDepthRaw = MeanDepthValue * GameConstants::DEPTH_VALUE_MULTIPLIER;
    const float MinDepthRaw = MinDepthValue * GameConstants::DEPTH_VALUE_MULTIPLIER;
    const float MaxDepthRaw = MaxDepthValue * GameConstants::DEPTH_VALUE_MULTIPLIER;

    LastThoraxDepth = MeanDepthRaw;
    bHasThoraxDepthReading = true;
    RecordThoraxDepthSample(MeanDepthRaw);

    //Sternum
    float SternumMeanDepthValue = 0.0f;
    int32 SternumSampleCount = 0;
    float DummyMin, DummyMax, DummyConf;
    
    if (bHasActiveSternumBounds && DepthSampler->ComputeMeanInBoundsUV(ActiveSternumMinUV, ActiveSternumMaxUV, SternumMeanDepthValue, SternumSampleCount, DummyMin, DummyMax, DummyConf))
    {
        LastSternumDepth = SternumMeanDepthValue * GameConstants::DEPTH_VALUE_MULTIPLIER;
        bHasSternumDepthReading = true;
        RecordSternumDepthSample(LastSternumDepth);
    }
    else
    {
        bHasSternumDepthReading = false;
    }

    // Zone grid del torace
    ComputeThoraxZoneDepths(ThoraxMinUV, ThoraxMaxUV);

    UpdateMainPanelDepth();
}

void AARHUD::DrawHUD()
{
    Super::DrawHUD();

    OverlayDrawer.DrawDepthToggleButton();
    OverlayDrawer.DrawChestSamplingArea();
    OverlayDrawer.DrawSternumArea();
    OverlayDrawer.DrawThoraxZoneDots();

    if (!Canvas || !PoseDetectorProvider.GetObject())
    {
        return;
    }

    if (bDrawJointDots || bDrawJointLabels)
    {
        OverlayDrawer.DrawJointsOverlay();
    }
}

void AARHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);

    if (BoxName == HUDConstants::DepthToggleHitBoxName)
    {
        bShowDepthOverlay = !bShowDepthOverlay;
        UpdateDepthWidgetState();
    }
}

// Methods migrated to HUDOverlayDrawer

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

bool AARHUD::TryGetSternumBoundsUV(FVector2D ThoraxMinUV, FVector2D ThoraxMaxUV, FVector2D& OutMinUV, FVector2D& OutMaxUV, float AreaSize) const
{
    // Calcoliamo la larghezza e l'altezza del torace in coordinate UV
    const float ThoraxWidth = ThoraxMaxUV.X - ThoraxMinUV.X;
    const float ThoraxHeight = ThoraxMaxUV.Y - ThoraxMinUV.Y;

    // Troviamo il centro dello sterno (metà delle U e 1/3 delle V dall'alto)
    const float SternumU = ThoraxMinUV.X + (ThoraxWidth * 0.5f);
    const float SternumV = ThoraxMinUV.Y + (ThoraxHeight * 0.5f);

    // Creiamo un'area quadrata attorno allo sterno proporzionale a SternumAreaSize
    const float BoxSize = ThoraxWidth * AreaSize;
    const float HalfBoxSize = BoxSize * 0.5f;

    OutMinUV.X = FMath::Clamp(SternumU - HalfBoxSize, 0.0f, 1.0f);
    OutMinUV.Y = FMath::Clamp(SternumV - HalfBoxSize, 0.0f, 1.0f);
    OutMaxUV.X = FMath::Clamp(SternumU + HalfBoxSize, 0.0f, 1.0f);
    OutMaxUV.Y = FMath::Clamp(SternumV + HalfBoxSize, 0.0f, 1.0f);

    return true;
}

void AARHUD::ComputeThoraxZoneDepths(FVector2D ThoraxMinUV, FVector2D ThoraxMaxUV)
{
    const int32 N = FMath::Max(1, NumThoraxZones);
    const int32 TotalZones = N * N;

    ThoraxZoneDepths.SetNum(TotalZones);
    for (float& V : ThoraxZoneDepths) { V = -1.0f; }

    const float URange = ThoraxMaxUV.X - ThoraxMinUV.X;
    const float VRange = ThoraxMaxUV.Y - ThoraxMinUV.Y;

    if (URange <= 0.0f || VRange <= 0.0f)
    {
        return;
    }

    const float CellU = URange / static_cast<float>(N);
    const float CellV = VRange / static_cast<float>(N);

    for (int32 Row = 0; Row < N; ++Row)
    {
        for (int32 Col = 0; Col < N; ++Col)
        {
            FVector2D ZoneMin, ZoneMax;
            ZoneMin.X = ThoraxMinUV.X + Col * CellU;
            ZoneMin.Y = ThoraxMinUV.Y + Row * CellV;
            ZoneMax.X = ZoneMin.X + CellU;
            ZoneMax.Y = ZoneMin.Y + CellV;

            float ZoneMean = 0.0f;
            int32 ZoneSamples = 0;
            float DummyMin, DummyMax, DummyConf;
            if (DepthSampler->ComputeMeanInBoundsUV(ZoneMin, ZoneMax, ZoneMean, ZoneSamples, DummyMin, DummyMax, DummyConf))
            {
                ThoraxZoneDepths[Row * N + Col] = ZoneMean * GameConstants::DEPTH_VALUE_MULTIPLIER;
            }
        }
    }

    if (bLogThoraxZoneDepths)
    {
        // ---- Costruiamo una stringa con tutti i valori in una sola stampa ----
        FString ZoneLog = FString::Printf(TEXT("[ThoraxZones %dx%d]"), N, N);
        for (int32 Row = 0; Row < N; ++Row)
        {
            ZoneLog += TEXT(" | Row") + FString::FromInt(Row) + TEXT(":");
            for (int32 Col = 0; Col < N; ++Col)
            {
                const float Val = ThoraxZoneDepths[Row * N + Col];
                if (Val < 0.0f)
                {
                    ZoneLog += TEXT(" [--]");
                }
                else
                {
                    ZoneLog += FString::Printf(TEXT(" [%.3f]"), Val);
                }
            }
        }
        UE_LOG(LogTemp, Log, TEXT("%s"), *ZoneLog);
    }
}



// Depth sampling methods migrated to UDepthSampler

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

void AARHUD::UpdateMainPanelState()
{
    if (!MainPanelWidget)
    {
        return;
    }

    MainPanelWidget->SetVisibility(
        bShowMainPanel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

// Drawing methods migrated to HUDOverlayDrawer

void AARHUD::RecordThoraxDepthSample(const float DepthUnits)
{
    if (!FMath::IsFinite(DepthUnits))
    {
        return;
    }

    ThoraxDepthHistory.Add(DepthUnits);

    const int32 MaxSamples = FMath::Max(1, ThoraxDepthHistoryMaxSamples);
    if (ThoraxDepthHistory.Num() > MaxSamples)
    {
        const int32 SamplesToTrim = ThoraxDepthHistory.Num() - MaxSamples;
        ThoraxDepthHistory.RemoveAt(0, SamplesToTrim, EAllowShrinking::No);
    }
}

void AARHUD::RecordSternumDepthSample(const float DepthUnits)
{
    if (!FMath::IsFinite(DepthUnits))
    {
        return;
    }

    SternumDepthHistory.Add(DepthUnits);

    const int32 MaxSamples = FMath::Max(1, ThoraxDepthHistoryMaxSamples);
    if (SternumDepthHistory.Num() > MaxSamples)
    {
        const int32 SamplesToTrim = SternumDepthHistory.Num() - MaxSamples;
        SternumDepthHistory.RemoveAt(0, SamplesToTrim, EAllowShrinking::No);
    }
}

void AARHUD::UpdateMainPanelDepth()
{
    if (!bEnableThoraxDepthGraphUpdates || !MainPanelWidget)
    {
        return;
    }

    MainPanelWidget->UpdateThoraxDepthGraph(ThoraxDepthHistory, LastThoraxDepth, bHasThoraxDepthReading);
    MainPanelWidget->UpdateSternumDepthGraph(SternumDepthHistory, LastSternumDepth, bHasSternumDepthReading);
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
