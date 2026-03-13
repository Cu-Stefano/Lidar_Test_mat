// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ScriptInterface.h"
#include "IDepthCamera.h"

class LIDAR_TEST_1_API CameraFactorySingleton
{

public:
    /**
     * Ottiene l'istanza singleton della factory
     */
    static CameraFactorySingleton& GetInstance();
    
    TScriptInterface<IIDepthCamera> CreateCamera(const FString& TypeName, UObject* Outer = nullptr);

    bool IsTypeSupported(const FString& TypeName) const;

    static TArray<FString> GetSupportedTypes();

private:

    // Lista dei tipi supportati
    static const TArray<FString> SupportedCameraTypes;

};
