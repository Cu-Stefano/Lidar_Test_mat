// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IDepthCamera.h"
#include "ARUnrealCamera.generated.h"


UCLASS()
class LIDAR_TEST_1_API UARUnrealCamera : public UObject, public IIDepthCamera
{
	GENERATED_BODY()

public:	
	UARUnrealCamera();

	virtual UTexture* GetDepthTexture() const override;
	virtual UTexture* GetCameraTexture() const override;
};
