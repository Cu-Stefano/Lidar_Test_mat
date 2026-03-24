// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AARHUD;

// -----------------------------------------------------------------------
// Costanti condivise HUD (usate sia da FHUDOverlayDrawer che da AARHUD)
// -----------------------------------------------------------------------
namespace HUDConstants
{
    inline const FName DepthToggleHitBoxName(TEXT("DepthToggleHitBox"));
}

/**
 * FHUDOverlayDrawer
 *
 * Plain C++ helper che raccoglie tutte le funzioni Draw* dell'HUD, in modo
 * da alleggerire AARHUD.cpp. Tiene un puntatore non-owning ad AARHUD e
 * accede ai suoi membri tramite la keyword friend dichiarata in ARHUD.h.
 */
class FHUDOverlayDrawer
{
public:
    explicit FHUDOverlayDrawer(AARHUD* InHUD) : HUD(InHUD) {}

    void DrawDepthToggleButton();
    void DrawChestSamplingArea();
    void DrawSternumArea();
    void DrawThoraxZoneDots();
    void DrawJointsOverlay();

private:
    AARHUD* HUD = nullptr;
};
