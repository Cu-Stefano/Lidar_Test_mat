// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Camera/ICameraWithDepth.h"
#include "ARUnrealCamera.generated.h"


UCLASS(Blueprintable, BlueprintType)
class IOX_API UARUnrealCamera : public UObject, public ICameraWithDepth
{
	GENERATED_BODY()

public:	

	virtual TObjectPtr<UTexture> GetDepthTexture() const override;
	virtual TObjectPtr<UTexture> GetCameraTexture() const override;
	virtual TObjectPtr<UTexture> GetConfidenceTexture() const override;
	virtual FVector2D GetCameraFocalLength() const override;

	virtual bool StartCamera() override;
	virtual void StopCamera() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<class UARSessionConfig> SessionConfig;
};
