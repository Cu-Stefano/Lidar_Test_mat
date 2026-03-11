// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PoseDetectionComponent.h"
#include "ARHUD.generated.h"

class UUserWidget;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UPoseDetectionComponent;
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
    void GetThoraxDepthHistory(TArray<int32>& OutHistoryMillimeters, int32& OutLatestDepthMillimeters, bool& bOutHasDepth) const;

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
    TSubclassOf<UUserWidget> DebugPanelClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> DebugPanelWidget = nullptr;

    UPROPERTY(EditAnywhere, Category="UI|DebugPanel")
    bool bShowDebugPanel = true;

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

    UPROPERTY(EditAnywhere, Category="UI|DepthReadout")
    bool bShowThoraxDepthReadout = false;

    UPROPERTY(EditAnywhere, Category="UI|DepthReadout")
    FVector2D ThoraxDepthReadoutPosition = FVector2D(24.0f, 84.0f);

    UPROPERTY(EditAnywhere, Category="UI|DepthReadout")
    FLinearColor ThoraxDepthReadoutColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, Category="UI|DepthReadout", meta=(ClampMin="0.5", ClampMax="3.0"))
    float ThoraxDepthReadoutScale = 4.0f;

    // ================= Rendering =================
    UPROPERTY(EditAnywhere, Category="Rendering")
    TObjectPtr<UTextureRenderTarget2D> CameraRenderTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rendering")
    TObjectPtr<UPoseDetectionComponent> PoseDetectionComponent = nullptr;

    // ================= Materiali =================
    UPROPERTY(EditAnywhere, Category="Rendering")
    TObjectPtr<UMaterialInterface> DepthMaterialBase = nullptr;

    UPROPERTY(EditAnywhere, Category="Rendering")
    TObjectPtr<UMaterialInterface> CameraMaterialBase = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DepthMaterial = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> CameraMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="Rendering")
    FName DepthTextureParameterName = TEXT("DepthParam");

    UPROPERTY(EditAnywhere, Category="Rendering")
    FName DepthNearMetersParameterName = TEXT("DepthNearMeters");

    UPROPERTY(EditAnywhere, Category="Rendering")
    FName DepthFarMetersParameterName = TEXT("DepthFarMeters");

    UPROPERTY(EditAnywhere, Category="Rendering", meta=(ClampMin="0.01", ClampMax="3.0"))
    float DepthNearMeters = 0.10f;

    UPROPERTY(EditAnywhere, Category="Rendering", meta=(ClampMin="0.5", ClampMax="20.0"))
    float DepthFarMeters = 9.0f;

    UPROPERTY(EditAnywhere, Category="Rendering")
    FName CameraTextureParameterName = TEXT("CameraTexture");

    // ================= Joint Overlay =================
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
    bool bDrawJointLabels = true;

    UPROPERTY(EditAnywhere, Category="Overlay")
    TObjectPtr<UFont> JointFont = nullptr;

    // ================= Depth Debug =================
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthMean = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="1", ClampMax="128"))
    int32 ThoraxDepthSampleRadiusPixels = 18;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthMinMax = true;

    // True when MT_DepthMaterial outputs normalized depth in [0..1] using DepthNearMeters/DepthFarMeters.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bDepthMaterialValuesAreNormalized = false;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bLogThoraxDepthInMillimeters = true;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug")
    bool bUseFloat32DepthSampling = true;

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
    float MaxValidDepthMeters = 10.0f;

    // Local standard deviation at this value or above is treated as low confidence.
    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.001", ClampMax="2.0"))
    float MaxDepthStdDevForConfidenceMeters = 0.18f;

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> DepthDebugRenderTarget = nullptr;

    float TimeSinceThoraxDepthLog = 0.0f;

    UPROPERTY(Transient)
    int32 LastThoraxDepthMillimeters = 0;

    UPROPERTY(Transient)
    bool bHasThoraxDepthReading = false;

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph", meta=(ClampMin="8", ClampMax="512"))
    int32 ThoraxDepthHistoryMaxSamples = 700;

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph")
    FName DebugPanelDepthUpdateFunctionName = TEXT("UpdateThoraxDepthGraph");

    UPROPERTY(EditAnywhere, Category="UI|DepthGraph")
    bool bEnableThoraxDepthGraphUpdates = true;

    UPROPERTY(Transient)
    TArray<int32> ThoraxDepthHistoryMillimeters;

    UPROPERTY(Transient)
    bool bLoggedMissingDepthGraphFunction = false;

private:
    void PushMaterialToWidget(UMaterialInterface* Material);
    void UpdateDepthWidgetState();
    void UpdateDebugPanelState();
    void DrawDepthToggleButton();
    void DrawThoraxDepthReadout();
    FVector2D ToScreenSpace(float X, float Y) const;
    void DrawJointsOverlay();
    void RecordThoraxDepthSample(int32 DepthMillimeters);
    void PushThoraxDepthToDebugPanel();
    bool TryGetThoraxDepthUV(const TArray<FPoseJoint>& Joints, FVector2D& OutUV) const;
    bool ComputeDepthMeanAtUV(
        const FVector2D& UV,
        float& OutMeanDepthValue,
        int32& OutSampleCount,
        float* OutMinDepthValue,
        float* OutMaxDepthValue,
        float* OutDepthSampleConfidence
    );
};
