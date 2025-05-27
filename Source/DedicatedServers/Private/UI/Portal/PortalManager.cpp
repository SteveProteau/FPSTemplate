// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Portal/PortalManager.h"

#include "HttpModule.h"
#include "Data/API/APIData.h"
#include "GameplayTags/DedicatedServersTags.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GameSessions/GameSessionsManager.h"

void UPortalManager::SignIn(const FString& Username, const FString& Password)
{
}

void UPortalManager::SignUp(const FString& Username, const FString& Password, const FString& Email)
{
	check(APIData);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServersTags::PortalAPI::SignUp);
	
	// Use shared ref because not a UE managed object
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	// Bind to response and send HTTP request
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::SignUp_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TMap<FString, FString> Params =
	{
		{TEXT("username"), Username}, 
		{TEXT("password"), Password},
		{TEXT("email"), Email}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UPortalManager::Confirm(const FString& ConfirmationCode)
{
}

void UPortalManager::QuitGame()
{
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (IsValid(LocalPlayerController))
	{
		UKismetSystemLibrary::QuitGame(this, LocalPlayerController, EQuitPreference::Quit, false);
	}
}

void UPortalManager::SignUp_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
}
