// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AARHUD;

namespace HUDConstants
{
    inline const FName DepthToggleHitBoxName(TEXT("DepthToggleHitBox"));
}

class FHUDOverlayDrawer
{

public:
    explicit FHUDOverlayDrawer(TObjectPtr<AARHUD> InHUD) : HUD(InHUD) {}

    void DrawChestSamplingArea();

private:
    TObjectPtr<AARHUD> HUD = nullptr;
};
