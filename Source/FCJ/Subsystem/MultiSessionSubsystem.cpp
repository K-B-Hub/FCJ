// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MultiSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "GameMode/LobbyGameState.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "PlayerController/MainMenuController.h"
#include "Online/OnlineSessionNames.h"
#include "GameFramework/PlayerState.h"

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
			sessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UMultiSessionSubsystem::OnLeaveSessionComplete);
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

	// 기존 세션이 있으면 먼저 파괴
	if (sessionInterface.IsValid())
	{
		FNamedOnlineSession* ExistingSession = sessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Destroying existing session before creating new one"));
			bPendingCreateServer = true; // 세션 파괴 후 재생성 플래그 설정
			sessionInterface->DestroySession(NAME_GameSession);
			return; // OnDestroySessionComplete에서 CreateServer를 다시 호출
		}
	}

	bPendingCreateServer = false; // 플래그 초기화

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

	// 기존 세션이 있으면 먼저 떠나기
	if (sessionInterface.IsValid())
	{
		FNamedOnlineSession* ExistingSession = sessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Leaving existing session before finding new one"));
			sessionInterface->EndSession(NAME_GameSession);
			return; // OnLeaveSessionComplete에서 FindServers를 다시 호출
		}
	}

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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("DestroyServer: Starting..."));

	// 모든 클라이언트 강퇴
	UWorld* World = GetWorld();
	if (World)
	{
		ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>();
		if (LobbyGS)
		{
			TArray<APlayerState*> PlayerArray = LobbyGS->PlayerArray;
			int32 ClientCount = 0;

			for (APlayerState* PS : PlayerArray)
			{
				if (PS)
				{
					AMainMenuController* ClientPC = Cast<AMainMenuController>(PS->GetOwner());
					if (ClientPC && !ClientPC->IsLocalController())
					{
						ClientCount++;
						// 클라이언트 연결 끊기
						ClientPC->ClientReturnToMainMenu();
					}
				}
			}

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
				FString::Printf(TEXT("DestroyServer: Kicked %d client(s)"), ClientCount));
		}
	}

	// 세션 파괴
	if (sessionInterface.IsValid())
	{
		sessionInterface->DestroySession(serverName);
	}
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// 모든 상태 초기화
	serverName = NAME_None;
	bInServer = false;

	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Session destroyed successfully."));

		// CreateServer가 기존 세션 파괴 후 재시도하는 경우, 다시 CreateServer 호출
		if (bPendingCreateServer)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Retrying CreateServer after destroying old session"));
			bPendingCreateServer = false;
			CreateServer();
		}
		else
		{
			TargetSessionId.Empty();
		}
	}
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

void UMultiSessionSubsystem::LeaveSession()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("LeaveSession"));

	if (sessionInterface.IsValid())
	{
		FNamedOnlineSession* Session = sessionInterface->GetNamedSession(NAME_GameSession);
		if (Session)
		{
			sessionInterface->EndSession(NAME_GameSession);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("No session to leave"));
		}
	}
}

void UMultiSessionSubsystem::OnLeaveSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("OnLeaveSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// 모든 상태 초기화
	bInServer = false;

	if (bWasSuccessful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Left session successfully. Ready to join new session."));

		// FindServers가 기존 세션 떠나기 후 재시도하는 경우, 다시 FindServers 호출
		if (!TargetSessionId.IsEmpty())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Retrying FindServers after leaving session"));
			FString SessionIdCopy = TargetSessionId;
			TargetSessionId.Empty();
			FindServers(SessionIdCopy);
		}
	}

	// 세션을 떠난 후에는 세션을 파괴
	if (sessionInterface.IsValid())
	{
		FNamedOnlineSession* Session = sessionInterface->GetNamedSession(SessionName);
		if (Session)
		{
			sessionInterface->DestroySession(SessionName);
		}
	}
}
