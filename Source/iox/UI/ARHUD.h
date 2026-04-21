// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UObject/ScriptInterface.h"
#include "Camera/ICameraWithDepth.h"
#include "Pose/IPoseDetector.h"
#include "Utils/Constants.h"
#include "Camera/DepthSampler.h"
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

UCLASS()
class IOX_API AARHUD : public AHUD
{
	GENERATED_BODY()

    friend class FHUDOverlayDrawer;

public:
    AARHUD();

    UFUNCTION(BlueprintCallable, Category="UI|DepthGraph")
    void GetThoraxDepthHistory(TArray<float>& OutHistory, float& OutLatestDepth, bool& bOutHasDepth) const;

    UFUNCTION(BlueprintCallable, Category="UI|DepthGraph")
    void GetSternumDepthHistory(TArray<float>& OutHistory, float& OutLatestDepth, bool& bOutHasDepth) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void DrawHUD() override;
    virtual void NotifyHitBoxClick(FName BoxName) override;

    // ================= UI =================
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> DepthWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> SceneDepthWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UMainPanel> MainPanelClass;

    UPROPERTY(Transient)
    TObjectPtr<UMainPanel> MainPanelWidget = nullptr;

    UPROPERTY(EditAnywhere, Category="UI|MainPanel")
    bool bShowMainPanel = true;

    UPROPERTY(EditAnywhere, Category="UI|DepthToggle")
    bool bShowDepthOverlay = false;

    UPROPERTY(EditAnywhere, Category="UI|DepthToggle")
    bool bShowDepthToggleButton = true;

    UPROPERTY(EditAnywhere, Category="UI|DepthToggle")
    FVector2D DepthToggleButtonPosition = FVector2D(24.0f, 24.0f);

    UPROPERTY(EditAnywhere, Category="UI|DepthToggle")
    FVector2D DepthToggleButtonSize = FVector2D(220.0f, 48.0f);

    UPROPERTY(EditAnywhere, Category="UI|DepthToggle")
    FLinearColor DepthToggleButtonOnColor = FLinearColor(0.10f, 0.45f, 0.18f, 0.90f);

    UPROPERTY(EditAnywhere, Category="UI|DepthToggle")
    FLinearColor DepthToggleButtonOffColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.90f);

    // ================= Rendering =================
    UPROPERTY(EditAnywhere, Category="UI")
    TObjectPtr<UTextureRenderTarget2D> CameraRenderTarget = nullptr;

    UPROPERTY(EditAnywhere, Category="UI")
    TObjectPtr<UTextureRenderTarget2D> DepthRenderTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
    TScriptInterface<IIPoseDetector> PoseDetectorProvider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
    TScriptInterface<ICameraWithDepth> CameraWithDepthProvider;

    // ================= Materiali =================
    UPROPERTY(EditAnywhere, Category="UI")
    TObjectPtr<UMaterialInterface> DepthMaterialBase = nullptr;

    UPROPERTY(EditAnywhere, Category="UI")
    TObjectPtr<UMaterialInterface> CameraMaterialBase = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DepthMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> CameraMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="UI")
    FName DepthTextureParameterName = TEXT("DepthParam");

    UPROPERTY(EditAnywhere, Category="UI")
    FName CameraTextureParameterName = TEXT("CameraTexture");

    // ================= Chest Sampling Overlay =================
    UPROPERTY(EditAnywhere, Category="Overlay|Chest")
    TObjectPtr<UMaterialInterface> ChestAreaMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="Overlay|Chest")
    bool bShowChestSamplingArea = true;

    UPROPERTY(EditAnywhere, Category="Overlay|Chest")
    FLinearColor ChestAreaFallbackColor = FLinearColor(0.2f, 0.9f, 0.3f, 0.25f);

    UPROPERTY(EditAnywhere, Category="Overlay|Chest")
    FLinearColor ChestAreaOutlineColor = FLinearColor(0.2f, 0.9f, 0.3f, 0.9f);

    UPROPERTY(EditAnywhere, Category="Overlay|Chest", meta=(ClampMin="0.5", ClampMax="8.0"))
    float ChestAreaOutlineThickness = 2.0f;

    // ================= Joint Overlay =================
    UPROPERTY(EditAnywhere, Category="Overlay")
    bool bDrawJointDots = false;

    UPROPERTY(EditAnywhere, Category="Overlay")
    TObjectPtr<UMaterialInterface> JointDotMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="Overlay")
    FLinearColor JointTextColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, Category="Overlay")
    float JointDotSize = 10.0f;

    UPROPERTY(EditAnywhere, Category="Overlay")
    float JointTextScale = 0.8f;

    UPROPERTY(EditAnywhere, Category="Overlay")
    float JointTextYOffset = 20.0f;

    UPROPERTY(EditAnywhere, Category="Overlay")
    float MinJointConfidence = 0.05f;

    UPROPERTY(EditAnywhere, Category="Overlay")
    bool bFlipNormalizedJointY = true;

    UPROPERTY(EditAnywhere, Category="Overlay")
    bool bDrawJointLabels = false;

    UPROPERTY(EditAnywhere, Category="Overlay")
    TObjectPtr<UFont> JointFont = nullptr;

    // ================= Depth Debug =================
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthMean = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthMinMax = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthIn = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bUseFloat32DepthSampling = false;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bUseDepthSampleConfidenceFilter = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinDepthSampleConfidence = 0.45f;

    // ================= Thorax Zone Grid =================
    UPROPERTY(EditAnywhere, Category="Pose|ThoraxZones", meta=(ClampMin="1", ClampMax="10"))
    int32 NumThoraxZones = 3;

    UPROPERTY(EditAnywhere, Category="Pose|ThoraxZones")
    bool bLogThoraxZoneDepths = true;

    UPROPERTY(EditAnywhere, Category="Pose|ThoraxZones")
    bool bDrawThoraxZoneDots = true;

    UPROPERTY(EditAnywhere, Category="Pose|ThoraxZones")
    FLinearColor ThoraxZoneDotColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category="Pose|ThoraxZones")
    float ThoraxZoneDotSize = 8.0f;
    
    float LastCalculatedTotalVolume = 0.0f;

    float LastVolumeFirstMax = 0.0f;
    float LastVolumeFirstMin = 0.0f;

    TArray<FThoraxZone> ThoraxZones;
    
    FVector2D ActiveThoraxMinUV = FVector2D::ZeroVector;

    FVector2D ActiveThoraxMaxUV = FVector2D::ZeroVector;

    bool bHasActiveThoraxBounds = false;

    UPROPERTY(Transient)
    TObjectPtr<UDepthSampler> DepthSampler = nullptr;

    float LastThoraxDepth = 0.0f;

    bool bHasThoraxDepthReading = false;

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph", meta=(ClampMin="8", ClampMax="1000"))
    int32 ThoraxDepthHistoryMaxSamples = 700;

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph")
    FName MainPanelDepthUpdateFunctionName = TEXT("UpdateThoraxDepthGraph");

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph")
    bool bEnableThoraxDepthGraphUpdates = true;

    TArray<float> ThoraxDepthHistory;

    bool CalculateInstantTotalVolume(float& OutTotalVolume);

    // ================= Sternum Sampling Overlay =================
    UPROPERTY(EditAnywhere, Category="Overlay|Sternum")
    TObjectPtr<UMaterialInterface> SternumAreaMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="Overlay|Sternum")
    bool bShowSternumSamplingArea = true;

    UPROPERTY(EditAnywhere, Category="Overlay|Sternum")
    FLinearColor SternumAreaFallbackColor = FLinearColor(1.0f, 0.1f, 0.1f, 0.25f);

    UPROPERTY(EditAnywhere, Category="Overlay|Sternum")
    FLinearColor SternumAreaOutlineColor = FLinearColor(1.0f, 0.1f, 0.1f, 0.9f);

    UPROPERTY(EditAnywhere, Category="Overlay|Sternum", meta=(ClampMin="0.5", ClampMax="8.0"))
    float SternumAreaOutlineThickness = 2.0f;

    // Sternum
    UPROPERTY(EditAnywhere, Category="UI|Sternum", meta=(ClampMin="0", ClampMax="1"))
    float SternumAreaSize = 0.5f;

    FVector2D ActiveSternumMinUV = FVector2D::ZeroVector;
    FVector2D ActiveSternumMaxUV = FVector2D::ZeroVector;
    bool bHasActiveSternumBounds = false;
    float LastSternumDepth = 0.0f;
    bool bHasSternumDepthReading = false;
    TArray<float> SternumDepthHistory;
    FHUDOverlayDrawer OverlayDrawer{this};

private:
    void ValidateEditorAssignments() const;
    void PushMaterialToWidget(TObjectPtr<UMaterialInterface> Material);
    void UpdateDepthWidgetState();
    void UpdateMainPanelState();
    void DrawDepthToggleButton();
    void DrawChestSamplingArea();
    void DrawSternumArea();
    void DrawThoraxZoneDots();
    FVector2D ToScreenSpace(float X, float Y) const;
    void DrawJointsOverlay();
    void RecordThoraxDepthSample(float DepthUnits);
    void RecordSternumDepthSample(float DepthUnits);
    void UpdateMainPanelDepth();
    bool TryGetThoraxBoundsUV(const TArray<FPoseJoint>& Joints, FVector2D& OutMinUV, FVector2D& OutMaxUV) const;
    bool TryGetSternumBoundsUV(FVector2D ThoraxMinUV, FVector2D ThoraxMaxUV, FVector2D& OutMinUV, FVector2D& OutMaxUV, float SternumAreaSize) const;
    bool CaptureDepthFrameData();
    bool ComputeDepthMeanInBoundsUV(
        const FVector2D& MinUV,
        const FVector2D& MaxUV,
        float& OutMeanDepthValue,
        int32& OutSampleCount,
        float& OutMinDepthValue,
        float& OutMaxDepthValue,
        float& OutDepthSampleConfidence
    );
    void ComputeThoraxZoneDepths(FVector2D ThoraxMinUV, FVector2D ThoraxMaxUV);
    bool CalculateThoraxTotalVolume(float& OutTotalVolume);
    EThoraxJointRole ResolveThoraxJointRole(const FString& RawName) const;
};
