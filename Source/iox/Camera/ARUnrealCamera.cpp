// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/ARUnrealCamera.h"
#include "ARBlueprintLibrary.h"
#include "ARTextures.h"
#include "ARSessionConfig.h"


TObjectPtr<UTexture> UARUnrealCamera::GetDepthTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap);
}

TObjectPtr<UTexture> UARUnrealCamera::GetCameraTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage);
}

TObjectPtr<UTexture> UARUnrealCamera::GetConfidenceTexture() const
{
	return UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthConfidenceMap);
}

FVector2D UARUnrealCamera::GetCameraFocalLength() const
{	
	FARCameraIntrinsics Intrinsics;
	if (UARBlueprintLibrary::GetCameraIntrinsics(Intrinsics))
	{
		return Intrinsics.FocalLength;
	}
	return FVector2D(-1.0f, .0f);
}

bool UARUnrealCamera::StartCamera()
{
	if (!SessionConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("UARUnrealCamera: Cannot start camera, SessionConfig is null."));
		return false;
	}

	UARBlueprintLibrary::StartARSession(SessionConfig);
	return true;
}

void UARUnrealCamera::StopCamera()
{
	UARBlueprintLibrary::StopARSession();
}

