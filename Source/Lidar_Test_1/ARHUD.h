// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UObject/ScriptInterface.h"
#include "ICameraWithDepth.h"
#include "IPoseDetector.h"
#include "ARHUD.generated.h"

class UUserWidget;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UFont;

/**
 * 
 */
UCLASS()
class LIDAR_TEST_1_API AARHUD : public AHUD
{
	GENERATED_BODY()

public:
    AARHUD();

    UFUNCTION(BlueprintCallable, Category="UI|DepthGraph")
    void GetThoraxDepthHistory(TArray<float>& OutHistoryMillimeters, float& OutLatestDepthMillimeters, bool& bOutHasDepth) const;

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
    TSubclassOf<UUserWidget> MainPanelClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> MainPanelWidget = nullptr;

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
    FName DepthNearMetersParameterName = TEXT("DepthNearMeters");

    UPROPERTY(EditAnywhere, Category="UI")
    FName DepthFarMetersParameterName = TEXT("DepthFarMeters");

    UPROPERTY(EditAnywhere, Category="UI", meta=(ClampMin="0.01", ClampMax="3.0"))
    float DepthNearMeters = 0.10f;

    UPROPERTY(EditAnywhere, Category="UI", meta=(ClampMin="0.5", ClampMax="20.0"))
    float DepthFarMeters = 9.0f;

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

    // True when MT_DepthMaterial outputs normalized depth in [0..1] using DepthNearMeters/DepthFarMeters.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bDepthMaterialValuesAreNormalized = false;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthInMillimeters = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bUseFloat32DepthSampling = false;

    // Reject depth readings when local sampling quality is poor.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bUseDepthSampleConfidenceFilter = true;

    // Confidence in [0..1], computed from valid-pixel ratio and local depth stability.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinDepthSampleConfidence = 0.45f;

    // Depth values outside this range are ignored as invalid during confidence estimation.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.01", ClampMax="20.0"))
    float MinValidDepthMeters = 0.05f;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.1", ClampMax="30.0"))
    float MaxValidDepthMeters = 1000000000.0f;

    // Local standard deviation at this value or above is treated as low confidence.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.001", ClampMax="2.0"))
    float MaxDepthStdDevForConfidenceMeters = 0.18f;

    UPROPERTY(Transient)
    FVector2D ActiveThoraxMinUV = FVector2D::ZeroVector;

    UPROPERTY(Transient)
    FVector2D ActiveThoraxMaxUV = FVector2D::ZeroVector;

    UPROPERTY(Transient)
    bool bHasActiveThoraxBounds = false;

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> DepthDebugRenderTarget = nullptr;

    UPROPERTY(Transient)
    float LastThoraxDepthMillimeters = 0.0f;

    UPROPERTY(Transient)
    bool bHasThoraxDepthReading = false;

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph", meta=(ClampMin="8", ClampMax="1000"))
    int32 ThoraxDepthHistoryMaxSamples = 700;

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph")
    FName MainPanelDepthUpdateFunctionName = TEXT("UpdateThoraxDepthGraph");

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph")
    bool bEnableThoraxDepthGraphUpdates = true;

    UPROPERTY(Transient)
    TArray<float> ThoraxDepthHistoryMillimeters;

    UPROPERTY(Transient)
    bool bLoggedMissingDepthGraphFunction = false;

private:
    void ValidateEditorAssignments() const;
    void PushMaterialToWidget(UMaterialInterface* Material);
    void UpdateDepthWidgetState();
    void UpdateMainPanelState();
    void DrawDepthToggleButton();
    void DrawChestSamplingArea();
    FVector2D ToScreenSpace(float X, float Y) const;
    void DrawJointsOverlay();
    void RecordThoraxDepthSample(float DepthUnits);
    void PushThoraxDepthToMainPanel();
    bool TryGetThoraxBoundsUV(const TArray<FPoseJoint>& Joints, FVector2D& OutMinUV, FVector2D& OutMaxUV) const;
    bool ComputeDepthMeanInBoundsUV(
        const FVector2D& MinUV,
        const FVector2D& MaxUV,
        float& OutMeanDepthValue,
        int32& OutSampleCount,
        float* OutMinDepthValue,
        float* OutMaxDepthValue,
        float* OutDepthSampleConfidence
    );
};
