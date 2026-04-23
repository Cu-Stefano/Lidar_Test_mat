// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "IOXSettings.generated.h"

/**
 * Global settings for the IOX project, accessible via Project Settings.
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="IOX Settings"))
class IOX_API UIOXSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UIOXSettings() : Super() {}

	virtual FName GetCategoryName() const override { return TEXT("Project"); }
	virtual FName GetSectionName() const override { return TEXT("IOX Settings"); }

	/** Map of camera type names to their corresponding Blueprint or C++ classes. */
	UPROPERTY(Config, EditAnywhere, Category="Camera", meta=(DisplayName="Camera Class Map"))
	TMap<FString, TSubclassOf<UObject>> CameraClassMap;

	/** Gets the global IOX settings instance. */
	static const UIOXSettings* Get() { return GetDefault<UIOXSettings>(); }
};
