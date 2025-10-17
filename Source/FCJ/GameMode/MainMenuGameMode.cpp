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

	// 디버그: 생성 시점 추적
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
		TEXT("🟢 MainMenuGameMode::BeginPlay() - GameMode Created"));

	// Apply saved display settings on game startup
	ApplySavedDisplaySettings();
}

void AMainMenuGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 디버그: 파괴 시점 추적
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		FString::Printf(TEXT("🔴 MainMenuGameMode::EndPlay() - Reason: %d"), static_cast<int32>(EndPlayReason)));

	Super::EndPlay(EndPlayReason);
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

	ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();
	if (!LobbyGS) return;

	// 세션 ID를 GameState에 설정
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
		if (SessionSubsystem)
		{
			FString CurrentSessionId = SessionSubsystem->GetCurrentSessionId();
			LobbyGS->SessionId = CurrentSessionId;
		}
	}

	// 플레이어를 LobbyGameState에 추가
	if (NewPlayer)
	{
		APlayerState* PS = NewPlayer->PlayerState;
		if (PS)
		{
			FUniqueNetIdRepl UniqueId = PS->GetUniqueId();
			FString PlayerNetId = UniqueId.IsValid() ? UniqueId->ToString() : TEXT("Unknown");
			FString PlayerName = PS->GetPlayerName();

			LobbyGS->AddPlayer(PlayerNetId, PlayerName);
		}

		// 로비 위젯 표시 (서버와 클라이언트 모두)
		AMainMenuController* MainMenuController = Cast<AMainMenuController>(NewPlayer);
		if (MainMenuController)
		{
			UMultiSessionSubsystem* Server = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
			if (NewPlayer->HasAuthority() && NewPlayer->IsLocalController() && Server && Server->bInServer)
			{
				// 서버(호스트)는 직접 로비 위젯 표시
				FString SessionId = Server->GetCurrentSessionId();
				MainMenuController->ShowLobbyWidget(SessionId);
			}
			else if (!NewPlayer->HasAuthority())
			{
				// 클라이언트는 RPC로 로비 위젯 표시
				MainMenuController->ClientShowLobbyWidget();
			}
		}
	}
}

void AMainMenuGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// 플레이어를 LobbyGameState에서 제거
	ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();
	if (LobbyGS && Exiting)
	{
		APlayerState* PS = Exiting->PlayerState;
		if (PS)
		{
			FUniqueNetIdRepl UniqueId = PS->GetUniqueId();
			FString PlayerNetId = UniqueId.IsValid() ? UniqueId->ToString() : TEXT("Unknown");

			// NetId로 플레이어 제거 (내부에서 역할 재할당도 처리됨)
			LobbyGS->RemovePlayer(PlayerNetId);
		}
	}
}

void AMainMenuGameMode::SwapPlayerRoles()
{
	ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>();
	if (LobbyGS)
	{
		LobbyGS->SwapPlayerRoles();
	}
}
