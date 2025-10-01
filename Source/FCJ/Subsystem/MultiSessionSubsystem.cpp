// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MultiSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "PlayerController/MainMenuController.h"
#include "Online/OnlineSessionNames.h"

#define NAME_GameSession FName(TEXT("GameSession"))

UMultiSessionSubsystem::UMultiSessionSubsystem()
{
	
}

void UMultiSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* os = IOnlineSubsystem::Get();
	if (os)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, os->GetSubsystemName().ToString());

		sessionInterface = os->GetSessionInterface();
		if (sessionInterface.IsValid())
		{
			sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiSessionSubsystem::OnCreateSessionComplete);
			sessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiSessionSubsystem::OnDestroySessionComplete);
			sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiSessionSubsystem::OnFindSessionsComplete);
			sessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiSessionSubsystem::OnJoinSessionComplete);
		}
	}
}

void UMultiSessionSubsystem::Deinitialize()
{
	Super::Deinitialize();

	
}

void UMultiSessionSubsystem::CreateServer()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("CreateServer"));
	
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bIsLANMatch = false;
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		SessionSettings.bIsLANMatch = true;		//NULL쓰면 true로
	}
	
	sessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UMultiSessionSubsystem::FindServers(FString SessionId)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("FindServers"));
	if (SessionId.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SessionId is empty"));
		return;
	}

	// 타겟 세션 ID 저장
	TargetSessionId = SessionId;

	// 세션 검색 설정
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	sessionSearch->MaxSearchResults = 100;
	sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	// 모든 세션 검색
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Searching for sessions..."));
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UMultiSessionSubsystem::DestroyServer()
{
	sessionInterface->DestroySession(serverName);
}

void UMultiSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));
	if (bWasSuccessful)
	{
		serverName = SessionName;
		bInServer = true;
		FNamedOnlineSession* NamedSession = sessionInterface->GetNamedSession(SessionName);
		if (NamedSession && NamedSession->SessionInfo.IsValid())
		{
			FString SessionIdStr = NamedSession->SessionInfo->GetSessionId().ToString();
			UE_LOG(LogTemp, Log, TEXT("Session ID: %s"), *SessionIdStr);

			// 리슨 서버로 전환 (로비 레벨로 이동)
			UWorld* World = GetWorld();
			if (World)
			{
				// MainMenu 레벨을 리슨 서버로 전환
				World->ServerTravel("/Game/Levels/MainMenu?listen");
			}
		}
	}
}

FString UMultiSessionSubsystem::GetCurrentSessionId() const
{
	if (sessionInterface.IsValid() && !serverName.IsNone())
	{
		FNamedOnlineSession* NamedSession = sessionInterface->GetNamedSession(serverName);
		if (NamedSession && NamedSession->SessionInfo.IsValid())
		{
			return NamedSession->SessionInfo->GetSessionId().ToString();
		}
	}
	return FString();
}

void UMultiSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	serverName = NAME_None;
	bInServer = false;
}

void UMultiSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("OnFindSessionsComplete: %d"), bWasSuccessful));

	if (!bWasSuccessful || !sessionSearch.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Session search failed"));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Found %d sessions"), sessionSearch->SearchResults.Num()));

	// 타겟 세션 ID와 일치하는 세션 찾기
	bool bFoundTargetSession = false;
	for (const FOnlineSessionSearchResult& Result : sessionSearch->SearchResults)
	{
		if (Result.Session.SessionInfo.IsValid())
		{
			FString CurrentSessionId = Result.Session.SessionInfo->GetSessionId().ToString();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Session ID: %s"), *CurrentSessionId));

			// 세션 ID 비교
			if (CurrentSessionId == TargetSessionId)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Target session found! Joining..."));
				bFoundTargetSession = true;

				// 세션 참가
				IOnlineSubsystem* os = IOnlineSubsystem::Get();
				if (os)
				{
					IOnlineIdentityPtr IdentityInterface = os->GetIdentityInterface();
					if (IdentityInterface.IsValid())
					{
						FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(0);
						if (LocalUserId.IsValid())
						{
							sessionInterface->JoinSession(*LocalUserId, NAME_GameSession, Result);
						}
					}
				}
				break;
			}
		}
	}

	if (!bFoundTargetSession)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Target session not found"));
	}
}

void UMultiSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("OnJoinSessionComplete: %d"), (int32)Result));

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Successfully joined session!"));

		// 세션 연결 정보 가져오기
		FString ConnectInfo;
		if (sessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Connect Info: %s"), *ConnectInfo));

			// 서버로 이동 (로비 위젯은 서버 연결 후 PostLogin에서 표시됨)
			UWorld* World = GetWorld();
			if (World)
			{
				APlayerController* PC = World->GetFirstPlayerController();
				if (PC)
				{
					PC->ClientTravel(ConnectInfo, TRAVEL_Absolute);
					bInServer = true;
				}
			}
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to join session"));
	}
}

void UMultiSessionSubsystem::SetPlayerRole(const FString& PlayerNetId, int32 Role)
{
	PlayerRoles.Add(PlayerNetId, Role);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
		FString::Printf(TEXT("Player %s assigned role: %d"), *PlayerNetId, Role));
}

int32 UMultiSessionSubsystem::GetPlayerRole(const FString& PlayerNetId) const
{
	const int32* Role = PlayerRoles.Find(PlayerNetId);
	return Role ? *Role : -1;
}

void UMultiSessionSubsystem::SwapPlayerRoles()
{
	if (PlayerRoles.Num() < 2) return;

	TArray<FString> PlayerIds;
	PlayerRoles.GetKeys(PlayerIds);

	if (PlayerIds.Num() >= 2)
	{
		int32 Role1 = PlayerRoles[PlayerIds[0]];
		int32 Role2 = PlayerRoles[PlayerIds[1]];

		PlayerRoles[PlayerIds[0]] = Role2;
		PlayerRoles[PlayerIds[1]] = Role1;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Player roles swapped in Subsystem"));
	}
}

void UMultiSessionSubsystem::ClearPlayerRoles()
{
	PlayerRoles.Empty();
}
