// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/ICameraWithDepth.h"
#include "MockCamera.generated.h"


UCLASS(Blueprintable, BlueprintType)
class IOX_API UMockCamera : public UObject, public ICameraWithDepth
{
	GENERATED_BODY()

public:	
	UMockCamera();

	virtual TObjectPtr<UTexture> GetDepthTexture() const override;
	virtual TObjectPtr<UTexture> GetCameraTexture() const override;
	virtual TObjectPtr<UTexture> GetConfidenceTexture() const override;
	virtual FVector2D GetCameraFocalLength() const override;

	virtual bool StartCamera() override { return true; }
	virtual void StopCamera() override {}

private:
	UPROPERTY(EditAnywhere, Category = "Mock")
	TObjectPtr<class UTexture2D> MockTexture;
};
