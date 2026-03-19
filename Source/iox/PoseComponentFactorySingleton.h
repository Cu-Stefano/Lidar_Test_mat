// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ScriptInterface.h"
#include "IPoseDetector.h"

class IOX_API PoseComponentFactorySingleton
{

public:

    static PoseComponentFactorySingleton& GetInstance();

    TScriptInterface<IIPoseDetector> CreatePoseComponent(const FString& TypeName, AActor* Owner = nullptr);

    bool IsTypeSupported(const FString& TypeName) const;

    static TArray<FString> GetSupportedTypes();

private:
    static const TArray<FString> SupportedPoseComponentTypes;
};
