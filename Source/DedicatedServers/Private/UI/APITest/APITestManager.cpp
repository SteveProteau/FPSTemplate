// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/APITest/APITestManager.h"

#include "HttpModule.h"
#include "Data/API/APIData.h"
#include "GameplayTags/DedicatedServersTags.h"
#include "Interfaces/IHttpResponse.h"

void UAPITestManager::ListFleetsButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "List Fleets button clicked");

	// Get HTTP URL for ListFleets
	check(APIData);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServersTags::GameSessionsAPI::ListFleets);
	
    // Use shared ref because not a UE managed object
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	// Bind to response and send HTTP request
	Request->OnProcessRequestComplete().BindUObject(this, &UAPITestManager::ListFleets_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "List Fleets Request Made");
}

void UAPITestManager::ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "List Fleets Request Received");

	// Deserialize JSON response into an Unreal Engine JSON object (not a UObject so we use a shared pointer).
	// The reader uses a shared reference because it cannot be null and is intialized immediately.
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (JsonObject->HasField(TEXT("FleetIds")))
		{
			for (const TSharedPtr<FJsonValue> Fleet : JsonObject->GetArrayField(TEXT("FleetIds")))
			{
				FString FleetString = Fleet->AsString();
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FleetString);
			}
		}
	}
}
