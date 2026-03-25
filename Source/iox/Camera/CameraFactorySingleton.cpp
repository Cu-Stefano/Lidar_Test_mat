// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/CameraFactorySingleton.h"
#include "Camera/ARUnrealCamera.h"
#include "Camera/MockCamera.h"
#include "Camera/ICameraWithDepth.h"

// Definizione dei dispositivi supportati
const TArray<FString> CameraFactorySingleton::SupportedCameraTypes = {
    TEXT("Unreal"),
    TEXT("Mock"),
};

CameraFactorySingleton& CameraFactorySingleton::GetInstance()
{
    static CameraFactorySingleton SingletonInstance;
    return SingletonInstance;
}

TScriptInterface<ICameraWithDepth> CameraFactorySingleton::CreateCamera(const FString& TypeName, TObjectPtr<UObject> Outer)
{
    TScriptInterface<ICameraWithDepth> Camera;
    UObject* EffectiveOuter = Outer ? Outer : GetTransientPackage();

    if (TypeName.Equals(TEXT("Unreal"), ESearchCase::IgnoreCase))
    {
        UARUnrealCamera* Provider = NewObject<UARUnrealCamera>(EffectiveOuter);
        Camera.SetObject(Provider);
        Camera.SetInterface(Cast<ICameraWithDepth>(Provider));
    }
    else if (TypeName.Equals(TEXT("Mock"), ESearchCase::IgnoreCase))
    {
        UMockCamera* Provider = NewObject<UMockCamera>(EffectiveOuter);
        Camera.SetObject(Provider);
        Camera.SetInterface(Cast<ICameraWithDepth>(Provider));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraFactorySingleton: Unsupported camera type '%s'"), *TypeName);
        return Camera;
    }

    if (Camera.GetObject())
    {
        UE_LOG(LogTemp, Log, TEXT("CameraFactorySingleton: Created camera of type '%s'"), *TypeName);
    }

    return Camera;
}

bool CameraFactorySingleton::IsTypeSupported(const FString& TypeName) const
{
    return SupportedCameraTypes.ContainsByPredicate([&TypeName](const FString& Type) {
        return Type.Equals(TypeName, ESearchCase::IgnoreCase);
    });
}

TArray<FString> CameraFactorySingleton::GetSupportedTypes()
{
    return SupportedCameraTypes;
}
