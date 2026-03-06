#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BodyPoseInterface.generated.h"


UINTERFACE(Blueprintable)
class UBodyPoseInterface : public UInterface
{
    GENERATED_BODY()
};

class IBodyPoseInterface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pose")
    int32 GetBodyCount() const;

};
