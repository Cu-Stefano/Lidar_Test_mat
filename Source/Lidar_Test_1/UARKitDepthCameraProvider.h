// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IDepthCamera.h"
#include "UARKitDepthCameraProvider.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIDAR_TEST_1_API UUARKitDepthCameraProvider : public UActorComponent, public IIDepthCamera
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUARKitDepthCameraProvider();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY(Transient) TObjectPtr<UTexture> DepthTexture = nullptr;
	UPROPERTY(Transient) TObjectPtr<UTexture> CameraTexture = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual UTexture* GetDepthTexture() const override;
	virtual UTexture* GetCameraTexture() const override;
	
};
