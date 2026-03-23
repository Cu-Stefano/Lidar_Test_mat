// Fill out your copyright notice in the Description page of Project Settings.


#include "MockCamera.h"
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

UTexture* UMockCamera::GetDepthTexture() const
{
	return MockTexture;
}

UTexture* UMockCamera::GetCameraTexture() const
{
	return MockTexture;
}
