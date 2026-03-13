// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ScriptInterface.h"
#include "IPoseDetector.h"

class LIDAR_TEST_1_API PoseComponentFactorySingleton
{

public:
    /**
     * Ottiene l'istanza singleton della factory.
     */
    static PoseComponentFactorySingleton& GetInstance();

    /**
     * Crea un componente pose detector del tipo richiesto e lo registra sull'Owner.
     * @param TypeName  Tipo da creare (e.g. "Default").
     * @param Owner     L'AActor a cui il componente verrà aggiunto.
     * @return          L'interfaccia IIPoseDetector del componente creato, non valida in caso di errore.
     */
    TScriptInterface<IIPoseDetector> CreatePoseComponent(const FString& TypeName, AActor* Owner = nullptr);

    bool IsTypeSupported(const FString& TypeName) const;

    static TArray<FString> GetSupportedTypes();

private:
    static const TArray<FString> SupportedPoseComponentTypes;
};
