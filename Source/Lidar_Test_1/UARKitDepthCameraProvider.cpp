// Fill out your copyright notice in the Description page of Project Settings.


#include "UARKitDepthCameraProvider.h"
#include "ARBlueprintLibrary.h"
#include "ARTextures.h"

// Sets default values for this component's properties
UUARKitDepthCameraProvider::UUARKitDepthCameraProvider()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUARKitDepthCameraProvider::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUARKitDepthCameraProvider::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DepthTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::SceneDepthMap);
	CameraTexture = UARBlueprintLibrary::GetARTexture(EARTextureType::CameraImage);
}

UTexture* UUARKitDepthCameraProvider::GetDepthTexture() const
{
	return DepthTexture.Get();
}

UTexture* UUARKitDepthCameraProvider::GetCameraTexture() const
{
	return CameraTexture.Get();
}

