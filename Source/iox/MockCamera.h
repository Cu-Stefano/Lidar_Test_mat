// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ICameraWithDepth.h"
#include "MockCamera.generated.h"


UCLASS()
class IOX_API UMockCamera : public UObject, public ICameraWithDepth
{
	GENERATED_BODY()

public:	
	UMockCamera();

	virtual UTexture* GetDepthTexture() const override;
	virtual UTexture* GetCameraTexture() const override;

private:
	UPROPERTY()
	class UTexture2D* MockTexture;
};
