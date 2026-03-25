// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/ICameraWithDepth.h"
#include "ARUnrealCamera.generated.h"


UCLASS()
class IOX_API UARUnrealCamera : public UObject, public ICameraWithDepth
{
	GENERATED_BODY()

public:	
	UARUnrealCamera();

	virtual TObjectPtr<UTexture> GetDepthTexture() const override;
	virtual TObjectPtr<UTexture> GetCameraTexture() const override;
};
