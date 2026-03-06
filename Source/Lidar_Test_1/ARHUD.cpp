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
#include "MyActorComponent.h"
#include "BodyPoseManager.h"

AARHUD::AARHUD()
{
    PrimaryActorTick.bCanEverTick = true;
    MyActorComponent = CreateDefaultSubobject<UMyActorComponent>(TEXT("MyActorComponent"));

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

    APlayerController* PC = GetOwningPlayerController();

    // =====================================================
    // 1) CREATE UI DEPTH SHOWER WIDGET + ADD TO VIEWPORT
    // =====================================================
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
        PushDepthMaterialToWidget();
    }

    if (MyActorComponent && CameraRenderTarget)
    {
        MyActorComponent->SetRenderTarget(CameraRenderTarget);
    }

    if (CameraMaterialBase)
    {
        CameraMaterial = UMaterialInstanceDynamic::Create(CameraMaterialBase, this);
    }
}

void AARHUD::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (DepthMaterial)
    {
        if (UARTexture* DepthTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap))
        {
            DepthMaterial->SetTextureParameterValue(DepthTextureParameterName, DepthTexture);
            PushDepthMaterialToWidget();
        }
    }

    if (CameraMaterial)
    {
        if (UARTexture* CameraTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage))
        {
            CameraMaterial->SetTextureParameterValue(CameraTextureParameterName, CameraTexture);
        }
    }

    if (CameraRenderTarget && CameraMaterial)
    {
        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, CameraRenderTarget, CameraMaterial);
    }

    if (MyActorComponent && CameraRenderTarget)
    {
        MyActorComponent->SetRenderTarget(CameraRenderTarget);
        MyActorComponent->OnTextureChange();
    }
}

void AARHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !MyActorComponent || !MyActorComponent->BodyPoseManager)
    {
        return;
    }

    const TArray<FPoseJoint>& Joints = MyActorComponent->BodyPoseManager->DetectedJoints;

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
            DrawText(
                Joint.Name,
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

void AARHUD::PushDepthMaterialToWidget()
{
    if (!SceneDepthWidget || !DepthMaterial)
    {
        return;
    }

    UFunction* SetImageMaterialFunction = SceneDepthWidget->FindFunction(TEXT("SetImageMaterial"));
    if (!SetImageMaterialFunction)
    {
        return;
    }

    struct FSetImageMaterialParams
    {
        UMaterialInterface* Material;
    };

    FSetImageMaterialParams Params;
    Params.Material = DepthMaterial;
    SceneDepthWidget->ProcessEvent(SetImageMaterialFunction, &Params);
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
