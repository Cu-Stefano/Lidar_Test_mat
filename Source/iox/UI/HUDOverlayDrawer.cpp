// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUDOverlayDrawer.h"
#include "UI/ARHUD.h"
#include "Pose/IPoseDetector.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"


void FHUDOverlayDrawer::DrawDepthToggleButton()
{
    if (!HUD->Canvas || !HUD->bShowDepthToggleButton)
    {
        return;
    }

    const FLinearColor FillColor = HUD->bShowDepthOverlay
        ? HUD->DepthToggleButtonOnColor
        : HUD->DepthToggleButtonOffColor;

    HUD->DrawRect(
        FillColor,
        HUD->DepthToggleButtonPosition.X,
        HUD->DepthToggleButtonPosition.Y,
        HUD->DepthToggleButtonSize.X,
        HUD->DepthToggleButtonSize.Y
    );

    const FString Label = HUD->bShowDepthOverlay ? TEXT("Depth: ON") : TEXT("Depth: OFF");
    HUD->DrawText(
        Label,
        FLinearColor::White,
        HUD->DepthToggleButtonPosition.X + 12.0f,
        HUD->DepthToggleButtonPosition.Y + 12.0f,
        HUD->JointFont,
        0.9f,
        false
    );

    HUD->AddHitBox(
        HUD->DepthToggleButtonPosition,
        HUD->DepthToggleButtonSize,
        HUDConstants::DepthToggleHitBoxName,
        true,
        0
    );
}

void FHUDOverlayDrawer::DrawChestSamplingArea()
{
    if (!HUD->Canvas || !HUD->bShowChestSamplingArea || !HUD->bHasActiveThoraxBounds)
    {
        return;
    }

    const float Width  = HUD->Canvas->ClipX;
    const float Height = HUD->Canvas->ClipY;
    if (Width <= 1.0f || Height <= 1.0f)
    {
        return;
    }

    const float MinX = FMath::Min(HUD->ActiveThoraxMinUV.X, HUD->ActiveThoraxMaxUV.X);
    const float MaxX = FMath::Max(HUD->ActiveThoraxMinUV.X, HUD->ActiveThoraxMaxUV.X);
    const float MinY = FMath::Min(HUD->ActiveThoraxMinUV.Y, HUD->ActiveThoraxMaxUV.Y);
    const float MaxY = FMath::Max(HUD->ActiveThoraxMinUV.Y, HUD->ActiveThoraxMaxUV.Y);

    float RectX = FMath::Clamp(MinX * Width,          0.0f, Width  - 1.0f);
    float RectY = FMath::Clamp(MinY * Height,         0.0f, Height - 1.0f);
    float RectW = FMath::Clamp((MaxX - MinX) * Width,  1.0f, Width  - RectX);
    float RectH = FMath::Clamp((MaxY - MinY) * Height, 1.0f, Height - RectY);

    if (HUD->ChestAreaMaterial)
    {
        HUD->DrawMaterial(
            HUD->ChestAreaMaterial,
            RectX, RectY, RectW, RectH,
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, false, 0.0f, FVector2D::ZeroVector
        );
    }
    else
    {
        HUD->DrawRect(HUD->ChestAreaFallbackColor, RectX, RectY, RectW, RectH);
    }

    if (HUD->ChestAreaOutlineThickness > 0.0f)
    {
        const float T = HUD->ChestAreaOutlineThickness;
        const FLinearColor C = HUD->ChestAreaOutlineColor;
        HUD->DrawLine(RectX,         RectY,         RectX + RectW, RectY,         C, T);
        HUD->DrawLine(RectX,         RectY + RectH, RectX + RectW, RectY + RectH, C, T);
        HUD->DrawLine(RectX,         RectY,         RectX,         RectY + RectH, C, T);
        HUD->DrawLine(RectX + RectW, RectY,         RectX + RectW, RectY + RectH, C, T);
    }
}

void FHUDOverlayDrawer::DrawSternumArea()
{
    if (!HUD->Canvas || !HUD->bShowSternumSamplingArea || !HUD->bHasActiveSternumBounds)
    {
        return;
    }

    const float Width  = HUD->Canvas->ClipX;
    const float Height = HUD->Canvas->ClipY;
    if (Width <= 1.0f || Height <= 1.0f)
    {
        return;
    }

    const float MinX = FMath::Min(HUD->ActiveSternumMinUV.X, HUD->ActiveSternumMaxUV.X);
    const float MaxX = FMath::Max(HUD->ActiveSternumMinUV.X, HUD->ActiveSternumMaxUV.X);
    const float MinY = FMath::Min(HUD->ActiveSternumMinUV.Y, HUD->ActiveSternumMaxUV.Y);
    const float MaxY = FMath::Max(HUD->ActiveSternumMinUV.Y, HUD->ActiveSternumMaxUV.Y);

    float RectX = FMath::Clamp(MinX * Width,          0.0f, Width  - 1.0f);
    float RectY = FMath::Clamp(MinY * Height,         0.0f, Height - 1.0f);
    float RectW = FMath::Clamp((MaxX - MinX) * Width,  1.0f, Width  - RectX);
    float RectH = FMath::Clamp((MaxY - MinY) * Height, 1.0f, Height - RectY);

    if (HUD->SternumAreaMaterial)
    {
        HUD->DrawMaterial(
            HUD->SternumAreaMaterial,
            RectX, RectY, RectW, RectH,
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, false, 0.0f, FVector2D::ZeroVector
        );
    }
    else
    {
        HUD->DrawRect(HUD->SternumAreaFallbackColor, RectX, RectY, RectW, RectH);
    }

    if (HUD->SternumAreaOutlineThickness > 0.0f)
    {
        const float T = HUD->SternumAreaOutlineThickness;
        const FLinearColor C = HUD->SternumAreaOutlineColor;
        HUD->DrawLine(RectX,         RectY,         RectX + RectW, RectY,         C, T);
        HUD->DrawLine(RectX,         RectY + RectH, RectX + RectW, RectY + RectH, C, T);
        HUD->DrawLine(RectX,         RectY,         RectX,         RectY + RectH, C, T);
        HUD->DrawLine(RectX + RectW, RectY,         RectX + RectW, RectY + RectH, C, T);
    }
}

void FHUDOverlayDrawer::DrawThoraxZoneDots()
{
    if (!HUD->Canvas || !HUD->bDrawThoraxZoneDots || !HUD->bHasActiveThoraxBounds)
    {
        return;
    }

    const int32 N      = FMath::Max(1, HUD->NumThoraxZones);
    const float URange = HUD->ActiveThoraxMaxUV.X - HUD->ActiveThoraxMinUV.X;
    const float VRange = HUD->ActiveThoraxMaxUV.Y - HUD->ActiveThoraxMinUV.Y;

    if (URange <= 0.0f || VRange <= 0.0f)
    {
        return;
    }

    const float Width   = HUD->Canvas->ClipX;
    const float Height  = HUD->Canvas->ClipY;
    const float CellU   = URange / static_cast<float>(N);
    const float CellV   = VRange / static_cast<float>(N);
    const float HalfDot = HUD->ThoraxZoneDotSize * 0.5f;

    for (int32 Row = 0; Row < N; ++Row)
    {
        for (int32 Col = 0; Col < N; ++Col)
        {
            const int32 ZoneIdx = Row * N + Col;
            if (!HUD->ThoraxZones.IsValidIndex(ZoneIdx))
            {
                continue;
            }

            const FThoraxZone& Zone = HUD->ThoraxZones[ZoneIdx];

            const float CenterU = (Zone.GetZoneMinUV().X + Zone.GetZoneMaxUV().X) * 0.5f;
            const float CenterV = (Zone.GetZoneMinUV().Y + Zone.GetZoneMaxUV().Y) * 0.5f;

            const FVector2D ScreenPos(CenterU * Width, CenterV * Height);

            // Grigio scuro se la zona non ha un valore depth valido.
            FLinearColor DrawColor = HUD->ThoraxZoneDotColor;
            if (Zone.GetLastDepth() < 0.0f)
            {
                DrawColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.6f);
            }

            if (HUD->JointDotMaterial && DrawColor == HUD->ThoraxZoneDotColor)
            {
                HUD->DrawMaterial(
                    HUD->JointDotMaterial,
                    ScreenPos.X - HalfDot, ScreenPos.Y - HalfDot,
                    HUD->ThoraxZoneDotSize, HUD->ThoraxZoneDotSize,
                    0.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, false
                );
            }
            else
            {
                HUD->DrawRect(DrawColor, ScreenPos.X - HalfDot, ScreenPos.Y - HalfDot,
                    HUD->ThoraxZoneDotSize, HUD->ThoraxZoneDotSize);
            }
        }
    }
}


void FHUDOverlayDrawer::DrawJointsOverlay()
{
    if (!HUD->bDrawJointDots && !HUD->bDrawJointLabels)
    {
        return;
    }

    IIPoseDetector* PoseDetector = HUD->PoseDetectorProvider.GetInterface();
    if (!PoseDetector)
    {
        return;
    }

    const TArray<FPoseJoint>& Joints = PoseDetector->GetDetectedJoints();
    if (Joints.Num() == 0)
    {
        return;
    }

    for (const FPoseJoint& Joint : Joints)
    {
        if (Joint.Confidence < HUD->MinJointConfidence)
        {
            continue;
        }

        const FVector2D ScreenPos = HUD->ToScreenSpace(Joint.X, Joint.Y);
        const float DotHalf = HUD->JointDotSize * 0.5f;
        const float DotX    = ScreenPos.X - DotHalf;
        const float DotY    = ScreenPos.Y - DotHalf;

        if (HUD->bDrawJointDots && HUD->JointDotMaterial)
        {
            HUD->DrawMaterial(
                HUD->JointDotMaterial,
                DotX, DotY,
                HUD->JointDotSize, HUD->JointDotSize,
                0.0f, 0.0f, 1.0f, 1.0f,
                1.0f, false, 0.0f, FVector2D::ZeroVector
            );
        }
        else if (HUD->bDrawJointDots)
        {
            HUD->DrawRect(HUD->JointTextColor, DotX, DotY, HUD->JointDotSize, HUD->JointDotSize);
        }

        if (HUD->bDrawJointLabels)
        {
            const FString JointLabel = Joint.Name.IsEmpty() ? TEXT("joint") : Joint.Name;
            HUD->DrawText(
                JointLabel,
                HUD->JointTextColor,
                ScreenPos.X,
                ScreenPos.Y + HUD->JointTextYOffset,
                HUD->JointFont,
                HUD->JointTextScale,
                false
            );
        }
    }
}
