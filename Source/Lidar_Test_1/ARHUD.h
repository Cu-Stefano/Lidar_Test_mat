// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyActorComponent.h"
#include "ARHUD.generated.h"

class UUserWidget;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UMyActorComponent;
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
    UUserWidget* SceneDepthWidget = nullptr;

    // ================= Rendering =================
    UPROPERTY(EditAnywhere, Category="Rendering")
    UTextureRenderTarget2D* CameraRenderTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rendering")
    UMyActorComponent* MyActorComponent = nullptr;

    // ================= Materiali =================
    UPROPERTY(EditAnywhere, Category="Rendering")
    UMaterialInterface* DepthMaterialBase = nullptr;

    UPROPERTY(EditAnywhere, Category="Rendering")
    UMaterialInterface* CameraMaterialBase = nullptr;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* DepthMaterial = nullptr;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* CameraMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category="Rendering")
    FName DepthTextureParameterName = TEXT("DepthParam");

    UPROPERTY(EditAnywhere, Category="Rendering")
    FName CameraTextureParameterName = TEXT("CameraTexture");

    // ================= Joint Overlay =================
    UPROPERTY(EditAnywhere, Category="Overlay")
    UMaterialInterface* JointDotMaterial = nullptr;

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
    UFont* JointFont = nullptr;

private:
    void PushDepthMaterialToWidget();
    FVector2D ToScreenSpace(float X, float Y) const;
};
