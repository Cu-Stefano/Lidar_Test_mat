// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ARHUD.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Pose/DefaultPoseDetector.h"
#include "Camera/CameraFactorySingleton.h"
#include "Pose/PoseComponentFactorySingleton.h"
#include "Camera/ICameraWithDepth.h"
#include "Graph/UDepthGraphWidget.h"
#include "UI/MainPanel.h"
#include "Utils/Constants.h"
#include "Camera/DepthSampler.h"
#include "Graph/GraphMath.h"
#include "Math/NumericLimits.h"

static const TMap<FString, EThoraxJointRole>& GetThoraxJointRoleDictionary()
{
    static const TMap<FString, EThoraxJointRole> Dictionary = {
        {TEXT("neck_1_joint"),          EThoraxJointRole::Torso},
        {TEXT("root"),                  EThoraxJointRole::Torso},
        {TEXT("left_shoulder_1_joint"), EThoraxJointRole::LeftShoulder},
        {TEXT("right_shoulder_1_joint"),EThoraxJointRole::RightShoulder},
        {TEXT("left_upleg_joint"),      EThoraxJointRole::LeftHip},
        {TEXT("right_upleg_joint"),     EThoraxJointRole::RightHip}
    };
    return Dictionary;
}

AARHUD::AARHUD()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AARHUD::BeginPlay()
{
    Super::BeginPlay();

    // Pose Component 
    PoseComponentFactorySingleton& PoseFactory = PoseComponentFactorySingleton::GetInstance();
    PoseDetectorProvider = PoseFactory.CreatePoseComponent(TEXT("Default"), this);

    if (!PoseDetectorProvider.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("AARHUD: PoseDetectorProvider creation failed."));
    }

    if (!CameraWithDepthProvider.GetObject())
    {
        CameraFactorySingleton& Factory = CameraFactorySingleton::GetInstance();
        CameraWithDepthProvider = Factory.CreateCamera(CameraTypeName, this);
    }

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

    if (CameraWithDepthProvider.GetObject())
    {
        CameraWithDepthProvider->StartCamera();
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
        if (TObjectPtr<UTexture> DepthTexture = CameraWithDepth->GetDepthTexture())
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
            if (TObjectPtr<UTexture> CameraTexture = CameraWithDepth->GetCameraTexture())
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

    TObjectPtr<UTexture> ConfidenceTexture = CameraWithDepth ? CameraWithDepth->GetConfidenceTexture() : nullptr;
    if (!DepthSampler || !DepthSampler->CaptureFrame(DepthMaterial, ConfidenceTexture, CameraRenderTarget, bUseFloat32DepthSampling))
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

    //Thorax
    const float SamplingMinConfidence = bUseDepthSampleConfidenceFilter ? MinDepthSampleConfidence : 0.0f;
    if (!DepthSampler->ComputeMeanInBoundsUV(ThoraxMinUV, ThoraxMaxUV, MeanDepthValue, SampleCount, MinDepthValue, MaxDepthValue, DepthSampleConfidence, SamplingMinConfidence))
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
    // Zone grid del torace
    ComputeThoraxZoneDepths(ThoraxMinUV, ThoraxMaxUV);

    RecordThoraxDepthSample(MeanDepthRaw);

    UpdateMainPanelDepth();
}
EThoraxJointRole AARHUD::ResolveThoraxJointRole(const FString& RawName) const
{
    const FString NameLower = RawName.ToLower();
    if (const EThoraxJointRole* Role = GetThoraxJointRoleDictionary().Find(NameLower))
    {
        return *Role;
    }
    return EThoraxJointRole::Unknown;
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
    const float ThoraxWidth = ThoraxMaxUV.X - ThoraxMinUV.X;
    const float ThoraxHeight = ThoraxMaxUV.Y - ThoraxMinUV.Y;

    const float SternumU = ThoraxMinUV.X + (ThoraxWidth * 0.5f);
    const float SternumV = ThoraxMinUV.Y + (ThoraxHeight * 0.5f);
 
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
    ICameraWithDepth* Camera = CameraWithDepthProvider.GetInterface();
    const FVector2D FocalLength = Camera ? Camera->GetCameraFocalLength() : FVector2D(-1.0f, -1.0f);
    const FVector2D Resolution = CameraRenderTarget ? FVector2D(CameraRenderTarget->SizeX, CameraRenderTarget->SizeY) : FVector2D::ZeroVector;

    const int32 N = FMath::Max(1, NumThoraxZones);
    const int32 TotalZones = N * N;

    if (ThoraxZones.Num() != TotalZones)
    {
        ThoraxZones.SetNum(TotalZones);
    }

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
            const int32 ZoneIdx = Row * N + Col;
            FThoraxZone& Zone = ThoraxZones[ZoneIdx];
            const FVector2D MinUVs = FVector2D(ThoraxMinUV.X + Col * CellU, ThoraxMinUV.Y + Row * CellV);
            const FVector2D MaxUVs = FVector2D(ThoraxMinUV.X + (Col + 1) * CellU, ThoraxMinUV.Y + (Row + 1) * CellV);

            Zone.UpdateBounds(MinUVs, MaxUVs, FocalLength, Resolution);

            float ZoneMean = 0.0f;
            int32 ZoneSamples = 0;
            float DummyMin, DummyMax, DummyConf;
            const float SamplingMinConfidence = bUseDepthSampleConfidenceFilter ? MinDepthSampleConfidence : 0.0f;
            if (DepthSampler->ComputeMeanInBoundsUV(MinUVs, MaxUVs, ZoneMean, ZoneSamples, DummyMin, DummyMax, DummyConf, SamplingMinConfidence))
            {
                Zone.AddDepthSample(ZoneMean * GameConstants::DEPTH_VALUE_MULTIPLIER, ThoraxDepthHistoryMaxSamples);
            }
            else
            {
                // Invalidate depth if calculation failed
                Zone.AddDepthSample(-1.0f, ThoraxDepthHistoryMaxSamples);
            }
        }
    }


    if (bLogThoraxZoneDepths)
    {
        if (MainPanelWidget)
        {
            MainPanelWidget->UpdateTotalVolume(AvgVolume);
        }
        UE_LOG(LogTemp, Log, TEXT("[ThoraxZones %dx%d Extrema | Total Volume: %.8f L]"), N, N, AvgVolume / 1000000.0f);   
    }
}

void AARHUD::PushMaterialToWidget(TObjectPtr<UMaterialInterface> Material)
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
        ThoraxHistoryBaseSampleIndex += SamplesToTrim;

        // Prune respiri che non possono piu' essere rappresentati nella history corrente.
        CachedBreathVolumes.RemoveAll([this](const FCachedBreathVolume& Breath)
        {
            return Breath.EndGlobalIndex < ThoraxHistoryBaseSampleIndex;
        });
    }
}


void AARHUD::UpdateMainPanelDepth()
{
    if (!bEnableThoraxDepthGraphUpdates || !MainPanelWidget || ThoraxDepthHistory.Num() < 3)
    {
        return;
    }

    if (ThoraxZones.Num() == 0)
    {
        return;
    }

    // Find global extrema on thorax history.
    TArray<float> XValues;
    XValues.SetNum(ThoraxDepthHistory.Num());
    for (int32 i = 0; i < XValues.Num(); ++i) XValues[i] = (float)i;
    TArray<GraphMath::FBreathPoint> ThoraxExtreme = GraphMath::FindExtrema(XValues, ThoraxDepthHistory, 0.05f, 30);
    
    if (ThoraxExtreme.Num() < 2)
    {
        MainPanelWidget->UpdateThoraxDepthGraph(ThoraxDepthHistory, TArray<float>(), LastThoraxDepth, bHasThoraxDepthReading);
        return;
    }

    const int32 PhaseCount = ThoraxExtreme.Num() - 1;
    TArray<float> TotalVolumes;
    TotalVolumes.SetNumZeroed(PhaseCount);

    const int32 BaseIndex = ThoraxHistoryBaseSampleIndex;
    const int32 IndexTolerance = FMath::Max(0, BreathIndexMatchTolerance);

    for (int32 PhaseIndex = 0; PhaseIndex < PhaseCount; ++PhaseIndex)
    {
        const int32 StartIndex = ThoraxExtreme[PhaseIndex].Index; //inizio del (r/e)espiro
        const int32 EndIndex = ThoraxExtreme[PhaseIndex + 1].Index; // fine (r/e)espiro

        if (StartIndex < 0 || EndIndex < 0 || EndIndex <= StartIndex)
        {
            continue;
        }

        const int32 StartGlobalIndex = BaseIndex + StartIndex;
        const int32 EndGlobalIndex = BaseIndex + EndIndex;

        // L'ultima fase di (r/e)espirazione ancora "attiva" (sarebbe l'ultimo punto, la fase attuale).
        const bool bIsFinalizedPhase = (PhaseIndex < (PhaseCount - 1));

        int32 CachedIndex = INDEX_NONE;
        if (bIsFinalizedPhase)
        {
            for (int32 i = 0; i < CachedBreathVolumes.Num(); ++i)
            {
                const FCachedBreathVolume& Cached = CachedBreathVolumes[i];
                const int32 StartDistance = FMath::Abs(Cached.StartGlobalIndex - StartGlobalIndex);
                const int32 EndDistance = FMath::Abs(Cached.EndGlobalIndex - EndGlobalIndex);

                if (StartDistance <= IndexTolerance && EndDistance <= IndexTolerance)
                {
                    CachedIndex = i;
                    break;
                }
            }

            if (CachedIndex == INDEX_NONE)
            {
                FCachedBreathVolume NewBreath;
                NewBreath.StartGlobalIndex = StartGlobalIndex;
                NewBreath.EndGlobalIndex = EndGlobalIndex;
                CachedIndex = CachedBreathVolumes.Add(NewBreath);
            }

            if (CachedBreathVolumes.IsValidIndex(CachedIndex) && CachedBreathVolumes[CachedIndex].bVolumeCalculated)
            {
                TotalVolumes[PhaseIndex] = CachedBreathVolumes[CachedIndex].VolumeMm3;
                continue;
            }
        }

        float PhaseVolume = 0.0f;
        for (const FThoraxZone& Zone : ThoraxZones)
        {
            PhaseVolume += Zone.GetVolumeBetweenIndexes(StartIndex, EndIndex);
        }

        TotalVolumes[PhaseIndex] = PhaseVolume;

        if (bIsFinalizedPhase && CachedBreathVolumes.IsValidIndex(CachedIndex))
        {
            FCachedBreathVolume& Cached = CachedBreathVolumes[CachedIndex];
            Cached.StartGlobalIndex = StartGlobalIndex;
            Cached.EndGlobalIndex = EndGlobalIndex;
            Cached.VolumeMm3 = PhaseVolume;
            Cached.bVolumeCalculated = true;
        }
    }

    //media di TotalVolumes
    float Avg = 0.0f;
    for (const float Volume : TotalVolumes)
    {
        Avg += Volume;
    }
    if (TotalVolumes.Num() > 0)
    {
        Avg /= TotalVolumes.Num();
    }
    AvgVolume = Avg;

    MainPanelWidget->UpdateThoraxDepthGraph(ThoraxDepthHistory, TotalVolumes, LastThoraxDepth, bHasThoraxDepthReading);
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
