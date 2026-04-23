// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUDOverlayDrawer.h"
#include "UI/ARHUD.h"
#include "Pose/IPoseDetector.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"




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
