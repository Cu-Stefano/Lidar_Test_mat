// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/ScriptInterface.h"
#include "Camera/ICameraWithDepth.h"
#include "Templates/SubclassOf.h"

class IOX_API CameraFactorySingleton
{

public:

    static CameraFactorySingleton& GetInstance();
    
    TScriptInterface<ICameraWithDepth> CreateCamera(const FString& TypeName, TObjectPtr<UObject> Outer = nullptr);
    TScriptInterface<ICameraWithDepth> CreateCameraByClass(TSubclassOf<UObject> CameraClass, TObjectPtr<UObject> Outer = nullptr);

    bool IsTypeSupported(const FString& TypeName) const;

    static TArray<FString> GetSupportedTypes();

private:

};
