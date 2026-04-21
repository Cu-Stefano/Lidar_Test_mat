#pragma once

#include "CoreMinimal.h"
#include "GraphTypes.generated.h"

USTRUCT(BlueprintType)
struct FGraphLabel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	int32 Index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	bool bIsPeak = false;

	FGraphLabel() = default;
	FGraphLabel(int32 InIndex, float InValue, bool bInIsPeak = false) 
		: Index(InIndex), Value(InValue), bIsPeak(bInIsPeak) {}
};
