// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/CameraFactorySingleton.h"
#include "Camera/ICameraWithDepth.h"
#include "Utils/IOXSettings.h"


CameraFactorySingleton& CameraFactorySingleton::GetInstance()
{
    static CameraFactorySingleton SingletonInstance;
    return SingletonInstance;
}

TScriptInterface<ICameraWithDepth> CameraFactorySingleton::CreateCamera(const FString& TypeName, TObjectPtr<UObject> Outer)
{
    TScriptInterface<ICameraWithDepth> Camera;
    TObjectPtr<UObject> EffectiveOuter = Outer ? Outer : GetTransientPackage();

    // 1. Check if there's a class mapped in Project Settings
    if (const UIOXSettings* Settings = UIOXSettings::Get())
    {
        if (const TSubclassOf<UObject>* ClassPtr = Settings->CameraClassMap.Find(TypeName))
        {
            if (*ClassPtr)
            {
                return CreateCameraByClass(*ClassPtr, EffectiveOuter);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CameraFactorySingleton: Unsupported camera type '%s'. Ensure it is configured in Project Settings -> IOX Settings."), *TypeName);
    return Camera;
}

TScriptInterface<ICameraWithDepth> CameraFactorySingleton::CreateCameraByClass(TSubclassOf<UObject> CameraClass, TObjectPtr<UObject> Outer)
{
    TScriptInterface<ICameraWithDepth> Camera;
    if (!CameraClass)
    {
        return Camera;
    }

    TObjectPtr<UObject> EffectiveOuter = Outer ? Outer : GetTransientPackage();
    TObjectPtr<UObject> NewCamera = NewObject<UObject>(EffectiveOuter, CameraClass);
    
    if (NewCamera && NewCamera->Implements<UCameraWithDepth>())
    {
        Camera.SetObject(NewCamera);
        Camera.SetInterface(Cast<ICameraWithDepth>(NewCamera));
        UE_LOG(LogTemp, Log, TEXT("CameraFactorySingleton: Created camera from class '%s'"), *CameraClass->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("CameraFactorySingleton: Class '%s' does not implement ICameraWithDepth"), *CameraClass->GetName());
    }

    return Camera;
}

bool CameraFactorySingleton::IsTypeSupported(const FString& TypeName) const
{
    if (const UIOXSettings* Settings = UIOXSettings::Get())
    {
        return Settings->CameraClassMap.Contains(TypeName);
    }
    return false;
}

TArray<FString> CameraFactorySingleton::GetSupportedTypes()
{
    TArray<FString> AllTypes;
    if (const UIOXSettings* Settings = UIOXSettings::Get())
    {
        Settings->CameraClassMap.GetKeys(AllTypes);
    }
    return AllTypes;
}
