// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/ARUnrealCamera.h"
#include "ARBlueprintLibrary.h"
#include "ARTextures.h"

UARUnrealCamera::UARUnrealCamera()
{
}

TObjectPtr<UTexture> UARUnrealCamera::GetDepthTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap);
}

TObjectPtr<UTexture> UARUnrealCamera::GetCameraTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage);
}

