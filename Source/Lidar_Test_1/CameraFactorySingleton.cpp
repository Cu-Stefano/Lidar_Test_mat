// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraFactorySingleton.h"
#include "ARUnrealCamera.h"
#include "IDepthCamera.h"

// Definizione dei dispositivi supportati
const TArray<FString> CameraFactorySingleton::SupportedCameraTypes = {
    TEXT("Unreal"),
};

CameraFactorySingleton& CameraFactorySingleton::GetInstance()
{
    static CameraFactorySingleton SingletonInstance;
    return SingletonInstance;
}

TScriptInterface<IIDepthCamera> CameraFactorySingleton::CreateCamera(const FString& TypeName, UObject* Outer)
{
    TScriptInterface<IIDepthCamera> Camera;
    UObject* EffectiveOuter = Outer ? Outer : GetTransientPackage();

    if (TypeName.Equals(TEXT("Unreal"), ESearchCase::IgnoreCase))
    {
        UARUnrealCamera* Provider = NewObject<UARUnrealCamera>(EffectiveOuter);
        Camera.SetObject(Provider);
        Camera.SetInterface(Cast<IIDepthCamera>(Provider));
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
