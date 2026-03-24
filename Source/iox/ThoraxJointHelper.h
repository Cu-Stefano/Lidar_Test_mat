// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UUserWidget;
class UUDepthGraphWidget;

// -----------------------------------------------------------------------
// Ruolo di un joint nel contesto del torace.
// -----------------------------------------------------------------------
enum class EThoraxJointRole : uint8
{
    Unknown,
    Torso,
    LeftShoulder,
    RightShoulder,
    LeftHip,
    RightHip,
};

/** Restituisce il mapping nome-joint → ruolo torace. */
const TMap<FString, EThoraxJointRole>& GetThoraxJointRoleDictionary();

/** Risolve il ruolo di un joint a partire dal nome grezzo. */
EThoraxJointRole ResolveThoraxJointRole(const FString& RawName);

/** Cerca ricorsivamente un UUDepthGraphWidget all'interno di un widget root. */
UUDepthGraphWidget* FindDepthGraphWidget(UUserWidget* RootWidget);
