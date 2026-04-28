#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Utils/SessionData.h"
#include "SessionExporter.generated.h"

UCLASS()
class IOX_API USessionExporter : public UObject
{
	GENERATED_BODY()

public:
    static void ExportSession(UObject* WorldContextObject, const FBreathingSessionData& SessionData);

    void SendSessionData(const FBreathingSessionData& SessionData);

private:
    FString SerializeSessionData(const FBreathingSessionData& SessionData) const;

    void SendPostRequest(const FString& Url, const FString& JsonPayload);

    void OnSessionDataSent(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
