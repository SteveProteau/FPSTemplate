// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameMode.h"

DEFINE_LOG_CATEGORY(LogShooterGameMode);

AShooterGameMode::AShooterGameMode()
{
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

#if WITH_GAMELIFT
	InitGameLift();
#endif
}

void AShooterGameMode::InitGameLift()
{
	UE_LOG(LogShooterGameMode, Log, TEXT("Initializing the GameLift Server"));

	FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

	// Define the server parameters for a GameLift Anywhere fleet (hosted on lur machine). These are
	// not needed for a GameLift managed EC2 fleet (Amazon servers).
	FServerParameters ServerParameters;

	// AuthToken returned from the "aws gamelift get-compute-auth-token" API. Note this will expire and require a new call to the API after 15 minutes.
	if (FParse::Value(FCommandLine::Get(), TEXT("-authtoken="), ServerParameters.m_authToken))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("AUTH_TOKEN: %s"), *ServerParameters.m_authToken)
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("-awsregion="), ServerParameters.m_awsRegion))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("AWS_REGION: %s"), *ServerParameters.m_awsRegion)
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("-accesskey="), ServerParameters.m_accessKey))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("ACCESS_KEY: %s"), *ServerParameters.m_accessKey)
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("-secretkey="), ServerParameters.m_secretKey))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("SECRET_KEY: % s"), *ServerParameters.m_secretKey)
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("-sessiontoken="), ServerParameters.m_sessionToken))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("SESSION_TOKEN: %s"), *ServerParameters.m_sessionToken)
	}

	// The Host/compute-name of the GameLift Anywhere instance.
	if (FParse::Value(FCommandLine::Get(), TEXT("-hostid="), ServerParameters.m_hostId))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("HOST_ID: %s"), *ServerParameters.m_hostId)
	}

	// The Anywhere Fleet ID.
	if (FParse::Value(FCommandLine::Get(), TEXT("-fleetid="), ServerParameters.m_fleetId))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("FLEET_ID: %s"), *ServerParameters.m_fleetId)
	}

	// The WebSocket URL (GameLiftServiceSdkEndpoint).
	if (FParse::Value(FCommandLine::Get(), TEXT("-websocketurl="), ServerParameters.m_webSocketUrl))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("WEBSOCKET_URL: %s"), *ServerParameters.m_webSocketUrl)
	}



}
