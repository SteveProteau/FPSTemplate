// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Portal/PortalManager.h"

#include "HttpModule.h"
#include "Data/API/APIData.h"
#include "GameplayTags/DedicatedServersTags.h"

void UPortalManager::JoinGameSession()
{
	BroadcastJoinGameSessionMessage.Broadcast(TEXT("Searching for a Game Session..."));
	
	check(APIData);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServersTags::GameSessionsAPI::FindOrCreateGameSession);
	
	// Use shared ref because not a UE managed object
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	// Bind to response and send HTTP request
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::FindOrCreateGameSession_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->ProcessRequest();
}

void UPortalManager::FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Find or Create Game Session Response Received");
}
