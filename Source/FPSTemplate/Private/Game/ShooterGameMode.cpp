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

void AShooterGameMode::SetServerParameters(FServerParameters& OutServerParameters)
{
	// AuthToken returned from the "aws gamelift get-compute-auth-token" API. Note this will expire and require a new call to the API after 15 minutes.
	if (FParse::Value(FCommandLine::Get(), TEXT("-authtoken="), OutServerParameters.m_authToken))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("AUTH_TOKEN: %s"), *OutServerParameters.m_authToken)
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("-awsregion="), OutServerParameters.m_awsRegion))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("AWS_REGION: %s"), *OutServerParameters.m_awsRegion)
	}

	if (FParse::Value(FCommandLine::Get(), TEXT("-accesskey="), OutServerParameters.m_accessKey))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("ACCESS_KEY: %s"), *OutServerParameters.m_accessKey)
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("-secretkey="), OutServerParameters.m_secretKey))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("SECRET_KEY: % s"), *OutServerParameters.m_secretKey)
	}
	if (FParse::Value(FCommandLine::Get(), TEXT("-sessiontoken="), OutServerParameters.m_sessionToken))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("SESSION_TOKEN: %s"), *OutServerParameters.m_sessionToken)
	}

	// The Host/compute-name of the GameLift Anywhere instance.
	if (FParse::Value(FCommandLine::Get(), TEXT("-hostid="), OutServerParameters.m_hostId))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("HOST_ID: %s"), *OutServerParameters.m_hostId)
	}

	// The Anywhere Fleet ID.
	if (FParse::Value(FCommandLine::Get(), TEXT("-fleetid="), OutServerParameters.m_fleetId))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("FLEET_ID: %s"), *OutServerParameters.m_fleetId)
	}

	// The WebSocket URL (GameLiftServiceSdkEndpoint).
	if (FParse::Value(FCommandLine::Get(), TEXT("-websocketurl="), OutServerParameters.m_webSocketUrl))
	{
		UE_LOG(LogShooterGameMode, Log, TEXT("WEBSOCKET_URL: %s"), *OutServerParameters.m_webSocketUrl)
	}

	// The PID of the running process
	FString ProcessId = "";
	if (FParse::Value(FCommandLine::Get(), TEXT("-processId="), ProcessId))
	{
		OutServerParameters.m_processId = TCHAR_TO_UTF8(*ProcessId);
	}
	else
	{
		// If no ProcessId is passed as a command line argument, generate a randomized unique string.
		FString TimeString = FString::FromInt(std::time(nullptr));
		ProcessId = "ProcessId_" + TimeString;
		OutServerParameters.m_processId = TCHAR_TO_UTF8(*ProcessId);
	}

	UE_LOG(LogShooterGameMode, Log, TEXT("PID: %s"), *OutServerParameters.m_processId);
}

void AShooterGameMode::InitGameLift()
{
	UE_LOG(LogShooterGameMode, Log, TEXT("Initializing the GameLift Server"));

	FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

	// Define the server parameters for a GameLift Anywhere fleet (hosted on lur machine). These are
	// not needed for a GameLift managed EC2 fleet (Amazon servers).
	FServerParameters ServerParameters;
	SetServerParameters(ServerParameters);

	// InitSDK establishes a local connection with GameLift's agent to enable further communication.
	//   Use InitSDK(ServerParameters) for a GameLift Anywhere fleet. 
	//   Use InitSDK() for a GameLift managed EC2 fleet.
	GameLiftSdkModule->InitSDK(ServerParameters);

	// Implement callback function OnStartGameSession
	//
    // GameLift sends a game session activation request to the game server
    // and passes a game session object with game properties and other settings.
    // Here is where a game server takes action based on the game session object.
    // When the game server is ready to receive incoming player connections, 
    // it invokes the server SDK call ActivateGameSession().
    auto OnStartGameSession = [=](Aws::GameLift::Server::Model::GameSession InGameSession)
    {
        FString GameSessionId = FString(InGameSession.GetGameSessionId());
        UE_LOG(LogShooterGameMode, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
		GameLiftSdkModule->ActivateGameSession();
    };

    ProcessParameters.OnStartGameSession.BindLambda(OnStartGameSession);

    // Implement callback function OnProcessTerminate
	//
    // GameLift invokes this callback before shutting down the instance hosting this game server.
    // It gives the game server a chance to save its state, communicate with services, etc., 
    // and initiate shut down. When the game server is ready to shut down, it invokes the 
    // server SDK call ProcessEnding() to tell GameLift it is shutting down.
    auto OnProcessTerminate = [=]()
    {
        UE_LOG(LogShooterGameMode, Log, TEXT("Game Server Process is terminating"));
        GameLiftSdkModule->ProcessEnding();
    };

    ProcessParameters.OnTerminate.BindLambda(OnProcessTerminate);

    // Implement callback function OnHealthCheck
	//
    // GameLift invokes this callback approximately every 60 seconds.
    // A game server might want to check the health of dependencies, etc.
    // Then it returns health status true if healthy, false otherwise.
    // The game server must respond within 60 seconds, or GameLift records 'false'.
    // In this example, the game server always reports healthy.
    auto OnHealthCheck = []()
    {
        UE_LOG(LogShooterGameMode, Log, TEXT("Performing Health Check"));
        return true;
    };

    ProcessParameters.OnHealthCheck.BindLambda(OnHealthCheck);

	// DIVERGED FROM COURSE (lesson 26)!
	//
	// The game server gets ready to report that it is ready to host game sessions
	// and that it will listen on the port for incoming player connections. The port
	// is optionally passed in as a command line parameter, or we use the default
	// port (typically 7777)
	if (!FParse::Value(FCommandLine::Get(), TEXT("-port="), ProcessParameters.port))
	{
		ProcessParameters.port = FURL::UrlConfig.DefaultPort;
	}
	
	// Here, the game server tells GameLift where to find game session log files.
	// At the end of a game session, GameLift uploads everything in the specified 
	// location and stores it in the cloud for access later.
	TArray<FString> Logfiles;
	Logfiles.Add(TEXT("FPSTemplate/Saved/Logs/FPSTemplate.log"));
	ProcessParameters.logParameters = Logfiles;

	// The game server calls ProcessReady() to tell GameLift it's ready to host game sessions.
	UE_LOG(LogShooterGameMode, Log, TEXT("Calling Process Ready"));
	GameLiftSdkModule->ProcessReady(ProcessParameters);
}
