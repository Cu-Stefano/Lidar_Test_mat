// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/ICameraWithDepth.h"
#include "MockCamera.generated.h"


UCLASS()
class IOX_API UMockCamera : public UObject, public ICameraWithDepth
{
	GENERATED_BODY()

public:	
	UMockCamera();

	virtual TObjectPtr<UTexture> GetDepthTexture() const override;
	virtual TObjectPtr<UTexture> GetCameraTexture() const override;
	virtual FVector2D GetCameraFocalLength() const override;

private:
	UPROPERTY()
	TObjectPtr<class UTexture2D> MockTexture;
};
