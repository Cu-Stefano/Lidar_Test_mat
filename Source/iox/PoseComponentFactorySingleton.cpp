// Fill out your copyright notice in the Description page of Project Settings.


#include "PoseComponentFactorySingleton.h"
#include "DefaultPoseDetector.h"

// Definizione dei tipi supportati
const TArray<FString> PoseComponentFactorySingleton::SupportedPoseComponentTypes = {
    TEXT("Default"),
};

PoseComponentFactorySingleton& PoseComponentFactorySingleton::GetInstance()
{
    static PoseComponentFactorySingleton SingletonInstance;
    return SingletonInstance;
}

TScriptInterface<IIPoseDetector> PoseComponentFactorySingleton::CreatePoseComponent(const FString& TypeName, AActor* Owner)
{
    TScriptInterface<IIPoseDetector> Result;

    if (!IsTypeSupported(TypeName))
    {
        UE_LOG(LogTemp, Warning, TEXT("PoseComponentFactorySingleton: Unsupported pose component type '%s'"), *TypeName);
        return Result;
    }

    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("PoseComponentFactorySingleton: Owner is null, cannot create pose detector."));
        return Result;
    }

    UDefaultPoseDetector* Component = NewObject<UDefaultPoseDetector>(
        Owner,
        UDefaultPoseDetector::StaticClass(),
        TEXT("DefaultPoseDetector")
    );

    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("PoseComponentFactorySingleton: Failed to create pose component of type '%s'"), *TypeName);
        return Result;
    }

    Result.SetObject(Component);
    Result.SetInterface(Cast<IIPoseDetector>(Component));

    UE_LOG(LogTemp, Log, TEXT("PoseComponentFactorySingleton: Created pose component of type '%s'"), *TypeName);
    return Result;
}

bool PoseComponentFactorySingleton::IsTypeSupported(const FString& TypeName) const
{
    return SupportedPoseComponentTypes.ContainsByPredicate([&TypeName](const FString& Type) {
        return Type.Equals(TypeName, ESearchCase::IgnoreCase);
    });
}

TArray<FString> PoseComponentFactorySingleton::GetSupportedTypes()
{
    return SupportedPoseComponentTypes;
}
