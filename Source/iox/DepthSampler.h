// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DepthSampler.generated.h"

class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;

/**
 * UDepthSampler
 *
 * Responsabile della cattura di un frame depth dalla GPU e del campionamento
 * UV su quell'ultimo frame. Staccato da AARHUD per separare la logica di
 * campionamento dal codice HUD.
 */
UCLASS()
class IOX_API UDepthSampler : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Cattura un frame depth rendendo il materiale su un render target interno.
     * Deve essere chiamato una sola volta per frame prima di ComputeMeanInBoundsUV.
     *
     * @param DepthMat       Material con il canale depth.
     * @param CameraRT       Render target della camera (usato per leggere le dimensioni).
     * @param bUseFloat32    true = RTF_RGBA32f, false = RTF_RGBA16f.
     * @return true se il frame è stato acquisito correttamente.
     */
    bool CaptureFrame(
        UMaterialInstanceDynamic* DepthMat,
        UTextureRenderTarget2D* CameraRT,
        bool bUseFloat32);

    /**
     * Calcola la media della depth in una regione [MinUV, MaxUV] sull'ultimo
     * frame catturato con CaptureFrame.
     */
    bool ComputeMeanInBoundsUV(
        const FVector2D& MinUV,
        const FVector2D& MaxUV,
        float& OutMeanDepthValue,
        int32& OutSampleCount,
        float& OutMinDepthValue,
        float& OutMaxDepthValue,
        float& OutDepthSampleConfidence) const;

private:
    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> DebugRenderTarget = nullptr;

    UPROPERTY(Transient)
    TArray<FLinearColor> FramePixels;

    int32 RTWidth = 0;
    int32 RTHeight = 0;
    bool bHasData = false;
};
