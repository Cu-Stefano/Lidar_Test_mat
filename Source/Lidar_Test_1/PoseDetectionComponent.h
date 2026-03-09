// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTextures.h"
#include "BodyPoseManager.h"
#include "PoseDetectionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIDAR_TEST_1_API UPoseDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
        UPoseDetectionComponent();

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pose|Debug")
        bool bPoseInputDropAlpha = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pose|Debug")
        bool bPoseInputSwapRedBlue = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pose|Debug")
        bool bPoseInputForceOpaqueAlpha = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pose|Debug")
        bool bPoseInputFlipY = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pose|Debug")
        bool bLogPoseInputStats = true;
        
        UPROPERTY(BlueprintReadOnly, Category="Pose")
        UBodyPoseManager* BodyPoseManager;

        UFUNCTION(BlueprintCallable, Category="AR")
        void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        UTextureRenderTarget2D* RenderTarget;
        
        UFUNCTION(BlueprintCallable, Category="AR")
        virtual void OnTextureChange() const;
protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    mutable bool bSavedCameraFrameOnce = false;
    
    TArray<uint8> GetRenderTargetBytes() const;
    
    int32 GetRenderTargetWidth() const;
    
    int32 GetRenderTargetHeight() const;

    bool ValidateRawBytes(const TArray<uint8>& RawBytes, int32 Width, int32 Height, int32 Channels, bool bEmitStats) const;

    TArray<uint8> ConvertRGBAtoRGB(const TArray<uint8>& RGBABytes) const;

    void FlipImageRowsInPlace(TArray<uint8>& Bytes, int32 Width, int32 Height, int32 Channels) const;

    void SaveRenderTargetToPNG() const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
