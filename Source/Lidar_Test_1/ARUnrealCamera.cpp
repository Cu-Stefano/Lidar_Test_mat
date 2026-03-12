// Fill out your copyright notice in the Description page of Project Settings.


#include "ARUnrealCamera.h"
#include "ARBlueprintLibrary.h"
#include "ARTextures.h"

UARUnrealCamera::UARUnrealCamera()
{
}

UTexture* UARUnrealCamera::GetDepthTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap);
}

UTexture* UARUnrealCamera::GetCameraTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage);
}

