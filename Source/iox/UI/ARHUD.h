// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UObject/ScriptInterface.h"
#include "Camera/ICameraWithDepth.h"
#include "Pose/IPoseDetector.h"
#include "Utils/Constants.h"
#include "UI/HUDOverlayDrawer.h"
#include "Graph/ThoraxZone.h"
#include "ARHUD.generated.h"

enum class EThoraxJointRole : uint8
{
    Unknown,
    Torso,
    LeftShoulder,
    RightShoulder,
    LeftHip,
    RightHip,
};

class UUserWidget;
class UMainPanel;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UFont;
class UDepthSampler;

UCLASS()
class IOX_API AARHUD : public AHUD
{
	GENERATED_BODY()

    friend class FHUDOverlayDrawer;

public:
    AARHUD();

    /** API for UI or external systems to get the current thorax data. */
    UFUNCTION(BlueprintCallable, Category="IOX|DepthGraph")
    void GetThoraxDepthHistory(TArray<float>& OutHistory, float& OutLatestDepth, bool& bOutHasDepth) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void DrawHUD() override;

private:
    // ================= UI Components =================
    UPROPERTY(EditDefaultsOnly, Category="IOX|UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> DepthWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> SceneDepthWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="IOX|UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UMainPanel> MainPanelClass;

    UPROPERTY(Transient)
    TObjectPtr<UMainPanel> MainPanelWidget = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|UI", meta=(AllowPrivateAccess="true"))
    bool bShowMainPanel = true;

    // ================= Depth Debug =================
    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true"))
    bool bLogThoraxDepthMean = true;

    UFUNCTION()
    TArray<FString> GetCameraTypeOptions() const;

    UPROPERTY(EditAnywhere, Category="IOX|Camera", meta=(AllowPrivateAccess="true", GetOptions="GetCameraTypeOptions"))
    FString CameraTypeName = TEXT("Unreal");

    UFUNCTION()
    TArray<FString> GetPoseDetectorOptions() const;

    UPROPERTY(EditAnywhere, Category="IOX|Pose", meta=(AllowPrivateAccess="true", GetOptions="GetPoseDetectorOptions"))
    FString PoseDetectorTypeName = TEXT("Default");

    // ================= Provider States =================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="IOX|Internal", meta=(AllowPrivateAccess="true"))
    TScriptInterface<IIPoseDetector> PoseDetectorProvider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="IOX|Internal", meta=(AllowPrivateAccess="true"))
    TScriptInterface<ICameraWithDepth> CameraWithDepthProvider;

    // ================= Rendering & Materials =================
    UPROPERTY(EditAnywhere, Category="IOX|Rendering", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UTextureRenderTarget2D> CameraRenderTarget = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|Rendering", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UTextureRenderTarget2D> DepthRenderTarget = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|Materials", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UMaterialInterface> DepthMaterialBase = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|Materials", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UMaterialInterface> CameraMaterialBase = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DepthMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> CameraMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|Materials", meta=(AllowPrivateAccess="true"))
    FName DepthTextureParameterName = TEXT("DepthParam");

    UPROPERTY(EditAnywhere, Category="IOX|Materials", meta=(AllowPrivateAccess="true"))
    FName CameraTextureParameterName = TEXT("CameraTexture");

    // ================= Thorax Sampling Overlay =================
    UPROPERTY(EditAnywhere, Category="IOX|Overlay", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UMaterialInterface> ChestAreaMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|Overlay", meta=(AllowPrivateAccess="true"))
    bool bShowChestSamplingArea = true;

    UPROPERTY(EditAnywhere, Category="IOX|Overlay", meta=(AllowPrivateAccess="true"))
    FLinearColor ChestAreaFallbackColor = FLinearColor(0.2f, 0.9f, 0.3f, 0.25f);

    UPROPERTY(EditAnywhere, Category="IOX|Overlay", meta=(AllowPrivateAccess="true"))
    FLinearColor ChestAreaOutlineColor = FLinearColor(0.2f, 0.9f, 0.3f, 0.9f);

    UPROPERTY(EditAnywhere, Category="IOX|Overlay", meta=(AllowPrivateAccess="true", ClampMin="0.5", ClampMax="8.0"))
    float ChestAreaOutlineThickness = 2.0f;

    // ================= UI Visibility =================
    UPROPERTY(EditAnywhere, Category="IOX|UI", meta=(AllowPrivateAccess="true"))
    bool bShowDepthOverlay = false;

    // ================= Sampling Internal State =================
    UPROPERTY(Transient)
    TObjectPtr<UDepthSampler> DepthSampler = nullptr;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true"))
    bool bUseFloat32DepthSampling = false;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true"))
    bool bUseDepthSampleConfidenceFilter = true;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
    float MinDepthSampleConfidence = 0.45f;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true"))
    float MinJointConfidence = 0.05f;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true"))
    bool bFlipNormalizedJointY = true;

    // ================= Thorax Zones & History =================
    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true", ClampMin="1", ClampMax="10"))
    int32 NumThoraxZones = 3;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true"))
    bool bLogThoraxZoneDepths = true;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true", ClampMin="8", ClampMax="1000"))
    int32 ThoraxDepthHistoryMaxSamples = 700;

    UPROPERTY(EditAnywhere, Category="IOX|UI", meta=(AllowPrivateAccess="true"))
    bool bEnableThoraxDepthGraphUpdates = true;

    UPROPERTY(EditAnywhere, Category="IOX|UI", meta=(AllowPrivateAccess="true"))
    FName MainPanelDepthUpdateFunctionName = TEXT("UpdateThoraxDepthGraph");

    TArray<FThoraxZone> ThoraxZones;
    TArray<float> ThoraxDepthHistory;
    float LastThoraxDepth = 0.0f;
    bool bHasThoraxDepthReading = false;

    FVector2D ActiveThoraxMinUV = FVector2D::ZeroVector;
    FVector2D ActiveThoraxMaxUV = FVector2D::ZeroVector;
    bool bHasActiveThoraxBounds = false;

    // ================= Volume Cache =================
    struct FCachedBreathVolume
    {
        int32 StartGlobalIndex = -1;
        int32 EndGlobalIndex = -1;
        float VolumeMm3 = 0.0f;
        bool bVolumeCalculated = false;
    };

    TArray<FCachedBreathVolume> CachedBreathVolumes;
    int32 ThoraxHistoryBaseSampleIndex = 0;
    float AvgVolume = 0.0f;

    UPROPERTY(EditAnywhere, Category="IOX|Sampling", meta=(AllowPrivateAccess="true", ClampMin="0", ClampMax="8"))
    int32 BreathIndexMatchTolerance = 4;

    FHUDOverlayDrawer OverlayDrawer{this};

    // ================= Internal Methods =================
    void PushMaterialToWidget(TObjectPtr<UMaterialInterface> Material);
    void UpdateDepthWidgetState();
    void UpdateMainPanelState();
    FVector2D ToScreenSpace(float X, float Y) const;
    void RecordThoraxDepthSample(float DepthUnits);
    void UpdateMainPanelDepth();

    bool TryGetThoraxBoundsUV(const TArray<FPoseJoint>& Joints, FVector2D& OutMinUV, FVector2D& OutMaxUV) const;
    void ComputeThoraxZoneDepths(FVector2D ThoraxMinUV, FVector2D ThoraxMaxUV);
    EThoraxJointRole ResolveThoraxJointRole(const FString& RawName) const;
};
