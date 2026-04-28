#include "Utils/SessionExporter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void USessionExporter::ExportSession(UObject* WorldContextObject, const FBreathingSessionData& SessionData)
{
    if (!WorldContextObject) return;

    USessionExporter* Exporter = NewObject<USessionExporter>(WorldContextObject);
    // Add to root to prevent GC while the request is in progress
    Exporter->AddToRoot();
    Exporter->SendSessionData(SessionData);
}

void USessionExporter::SendSessionData(const FBreathingSessionData& SessionData)
{
    FString JsonPayload = SerializeSessionData(SessionData);
    
    SendPostRequest(TEXT("https://iox.vectorlab-cg.com/submit"), JsonPayload);
}

FString USessionExporter::SerializeSessionData(const FBreathingSessionData& SessionData) const
{
    // Create JSON object
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("title"), SessionData.Title);
    JsonObject->SetStringField(TEXT("description"), SessionData.Description);

    TArray<TSharedPtr<FJsonValue>> LabelsArray;
    for (const FString& L : SessionData.Labels) LabelsArray.Add(MakeShareable(new FJsonValueString(L)));
    JsonObject->SetArrayField(TEXT("labels"), LabelsArray);

    TArray<TSharedPtr<FJsonValue>> ValuesArray;
    for (float V : SessionData.Values) ValuesArray.Add(MakeShareable(new FJsonValueNumber(V)));
    JsonObject->SetArrayField(TEXT("values"), ValuesArray);

    TArray<TSharedPtr<FJsonValue>> AnnotationsArray;
    for (const FBreathingAnnotation& Ann : SessionData.Annotations)
    {
        TArray<TSharedPtr<FJsonValue>> AnnEntry;
        AnnEntry.Add(MakeShareable(new FJsonValueNumber(Ann.Index)));
        AnnEntry.Add(MakeShareable(new FJsonValueString(Ann.Label)));
        AnnotationsArray.Add(MakeShareable(new FJsonValueArray(AnnEntry)));
    }
    JsonObject->SetArrayField(TEXT("annotations"), AnnotationsArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    return OutputString;
}

void USessionExporter::SendPostRequest(const FString& Url, const FString& JsonPayload)
{
    // Send HTTP Request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &USessionExporter::OnSessionDataSent);
    
    Request->SetURL(Url);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("User-Agent"), TEXT("UnrealEngine/iox-agent"));
    Request->SetHeader(TEXT("Cache-Control"), TEXT("no-cache"));
    
    Request->SetContentAsString(JsonPayload);
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("USessionExporter: Tentativo di invio a %s"), *Url);
    UE_LOG(LogTemp, Log, TEXT("USessionExporter: Payload inviato (lunghezza: %d)."), JsonPayload.Len());
}

void USessionExporter::OnSessionDataSent(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("USessionExporter: Sessione inviata con successo: %d - %s"), Response->GetResponseCode(), *Response->GetContentAsString());
    }
    else
    {
        int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : -1;
        FString ErrorStr = bWasSuccessful ? TEXT("Response Invalid") : TEXT("Connection Failed");
        UE_LOG(LogTemp, Warning, TEXT("USessionExporter: Invio sessione fallito. Code: %d, Status: %s"), ResponseCode, *ErrorStr);
    }

    // Remove from root to allow GC
    RemoveFromRoot();
}
