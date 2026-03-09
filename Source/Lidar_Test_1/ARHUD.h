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

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void DrawHUD() override;

    // ================= UI =================
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> DepthWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> SceneDepthWidget = nullptr;

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

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="0.05", ClampMax="5.0"))
    float ThoraxDepthLogIntervalSeconds = 0.25f;

    UPROPERTY(EditAnywhere, Category="Pose|DepthDebug", meta=(ClampMin="1", ClampMax="128"))
    int32 ThoraxDepthSampleRadiusPixels = 18;

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> DepthDebugRenderTarget = nullptr;

    float TimeSinceThoraxDepthLog = 0.0f;

private:
    void PushDepthMaterialToWidget();
    FVector2D ToScreenSpace(float X, float Y) const;
    void DrawJointsOverlay();
    bool TryGetThoraxDepthUV(const TArray<FPoseJoint>& Joints, FVector2D& OutUV) const;
    bool ComputeDepthMeanAtUV(const FVector2D& UV, float& OutMeanDepth01, int32& OutSampleCount);
};
