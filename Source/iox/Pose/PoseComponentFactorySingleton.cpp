// Fill out your copyright notice in the Description page of Project Settings.


#include "Pose/PoseComponentFactorySingleton.h"
#include "Pose/DefaultPoseDetector.h"
#include "Utils/IOXSettings.h"


PoseComponentFactorySingleton& PoseComponentFactorySingleton::GetInstance()
{
    static PoseComponentFactorySingleton SingletonInstance;
    return SingletonInstance;
}

TScriptInterface<IIPoseDetector> PoseComponentFactorySingleton::CreatePoseComponent(const FString& TypeName, TObjectPtr<AActor> Owner)
{
    TScriptInterface<IIPoseDetector> Result;

    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("PoseComponentFactorySingleton: Owner is null, cannot create pose detector."));
        return Result;
    }

    // 1. Check Project Settings
    if (const UIOXSettings* Settings = UIOXSettings::Get())
    {
        if (const TSubclassOf<UObject>* ClassPtr = Settings->PoseClassMap.Find(TypeName))
        {
            if (*ClassPtr)
            {
                return CreatePoseComponentByClass(*ClassPtr, Owner);
            }
        }
    }

    // 2. Fallback to hardcoded default
    if (TypeName.Equals(TEXT("Default"), ESearchCase::IgnoreCase))
    {
        TObjectPtr<UDefaultPoseDetector> Component = NewObject<UDefaultPoseDetector>(
            Owner,
            UDefaultPoseDetector::StaticClass(),
            TEXT("DefaultPoseDetector")
        );

        if (Component)
        {
            Result.SetObject(Component);
            Result.SetInterface(Cast<IIPoseDetector>(Component));
            return Result;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("PoseComponentFactorySingleton: Unsupported pose component type '%s'"), *TypeName);
    return Result;
}

TScriptInterface<IIPoseDetector> PoseComponentFactorySingleton::CreatePoseComponentByClass(TSubclassOf<UObject> PoseClass, TObjectPtr<AActor> Owner)
{
    TScriptInterface<IIPoseDetector> Result;
    if (!PoseClass || !Owner) return Result;

    UObject* NewDetector = NewObject<UObject>(Owner, PoseClass);
    if (NewDetector && NewDetector->Implements<UIPoseDetector>())
    {
        Result.SetObject(NewDetector);
        Result.SetInterface(Cast<IIPoseDetector>(NewDetector));
    }
    return Result;
}

bool PoseComponentFactorySingleton::IsTypeSupported(const FString& TypeName) const
{
    if (TypeName.Equals(TEXT("Default"), ESearchCase::IgnoreCase)) return true;

    if (const UIOXSettings* Settings = UIOXSettings::Get())
    {
        return Settings->PoseClassMap.Contains(TypeName);
    }
    return false;
}

TArray<FString> PoseComponentFactorySingleton::GetSupportedTypes()
{
    TArray<FString> AllTypes;
    AllTypes.Add(TEXT("Default"));

    if (const UIOXSettings* Settings = UIOXSettings::Get())
    {
        for (const auto& Pair : Settings->PoseClassMap)
        {
            AllTypes.AddUnique(Pair.Key);
        }
    }
    return AllTypes;
}
