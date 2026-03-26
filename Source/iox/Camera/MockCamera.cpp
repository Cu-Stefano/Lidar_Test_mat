// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/MockCamera.h"
#include "ARTextures.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"

UMockCamera::UMockCamera()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> TextureAsset(TEXT("/Game/Materials/test"));
	if (TextureAsset.Succeeded())
	{
		MockTexture = TextureAsset.Object;
	}
}

TObjectPtr<UTexture> UMockCamera::GetDepthTexture() const
{
	return MockTexture;
}

TObjectPtr<UTexture> UMockCamera::GetCameraTexture() const
{
	return MockTexture;
}

FVector2D UMockCamera::GetCameraFocalLength() const
{
	return FVector2D(1000.0f, 1000.0f);
}
