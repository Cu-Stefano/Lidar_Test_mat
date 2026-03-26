// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "UObject/Interface.h"
#include "ICameraWithDepth.generated.h"

class UTexture;

UINTERFACE(MinimalAPI)
class UCameraWithDepth : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IOX_API ICameraWithDepth
{
	GENERATED_BODY()

public:

    virtual TObjectPtr<UTexture> GetDepthTexture() const = 0;
    virtual TObjectPtr<UTexture> GetCameraTexture() const = 0;
    virtual FVector2D GetCameraFocalLength() const = 0;
};
