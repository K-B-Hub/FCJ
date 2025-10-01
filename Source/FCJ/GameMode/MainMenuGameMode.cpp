// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "FCJ/PlayerController/MainMenuController.h"
#include "FCJ/Widdget/MainMenuWidget.h"
#include "FCJ/Subsystem/MultiSessionSubsystem.h"
#include "FCJ/GameMode/LobbyGameState.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerState.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	PlayerControllerClass = AMainMenuController::StaticClass();
	GameStateClass = ALobbyGameState::StaticClass();
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Apply saved display settings on game startup
	ApplySavedDisplaySettings();
}

void AMainMenuGameMode::ApplySavedDisplaySettings()
{
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (GameUserSettings)
	{
		GameUserSettings->LoadSettings();
		GameUserSettings->ApplySettings(false);
	}
}

void AMainMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 세션 ID를 GameState에 설정
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
		if (SessionSubsystem)
		{
			FString CurrentSessionId = SessionSubsystem->GetCurrentSessionId();
			ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();
			if (LobbyGS)
			{
				LobbyGS->SessionId = CurrentSessionId;
			}
		}
	}

	// 로비 위젯 표시 (서버와 클라이언트 모두)
	if (NewPlayer)
	{
		AMainMenuController* MainMenuController = Cast<AMainMenuController>(NewPlayer);
		if (MainMenuController)
		{
			UMultiSessionSubsystem* Server = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
			if (NewPlayer->HasAuthority() && NewPlayer->IsLocalController())
			{
				// 서버(호스트)는 직접 로비 위젯 표시
				FString SessionId = Server ? Server->GetCurrentSessionId() : FString();
				MainMenuController->ShowLobbyWidget(SessionId);
			}
			else
			{
				// 클라이언트는 RPC로 로비 위젯 표시
				MainMenuController->ClientShowLobbyWidget();
			}
		}
	}

	// GameState 역할 정보 업데이트
	UpdateGameStateRoles();
}

void AMainMenuGameMode::SwapPlayerRoles()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
	if (!SessionSubsystem) return;

	// Subsystem에서 역할 교체
	SessionSubsystem->SwapPlayerRoles();

	// GameState 업데이트
	UpdateGameStateRoles();
}

void AMainMenuGameMode::UpdateGameStateRoles()
{
	ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();
	if (!LobbyGS) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
	if (!SessionSubsystem) return;

	TArray<FPlayerRoleInfo> RoleInfos;

	// 모든 플레이어의 역할 정보 수집
	for (APlayerState* PS : LobbyGS->PlayerArray)
	{
		if (PS)
		{
			FUniqueNetIdRepl UniqueId = PS->GetUniqueId();
			if (UniqueId.IsValid())
			{
				FString PlayerNetId = UniqueId->ToString();
				int32 role = SessionSubsystem->GetPlayerRole(PlayerNetId);

				// 역할이 설정되지 않았으면 새로 할당
				if (role == -1)
				{
					role = RoleInfos.Num(); // 0, 1 순서대로 할당
					SessionSubsystem->SetPlayerRole(PlayerNetId, role);
				}

				FPlayerRoleInfo Info(PS->GetPlayerName(), role);
				RoleInfos.Add(Info);

				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
					FString::Printf(TEXT("UpdateGameState: %s -> Role %d"), *PS->GetPlayerName(), role));
			}
		}
	}

	// GameState에 업데이트 (리플리케이트됨)
	LobbyGS->UpdatePlayerRoles(RoleInfos);
}
