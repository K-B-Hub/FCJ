// Fill out your copyright notice in the Description page of Project Settings.


#include "Widdget/LobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameMode/MultiGameMode.h"
#include "GameMode/MainMenuGameMode.h"
#include "GameMode/LobbyGameState.h"
#include "Subsystem/MultiSessionSubsystem.h"
#include "PlayerController/MainMenuController.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformApplicationMisc.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SwapRolesButton)
	{
		SwapRolesButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnSwapRolesClicked);
	}

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartGameClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnBackClicked);
	}

	if (CopySessionIdButton)
	{
		CopySessionIdButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnCopySessionIdClicked);
	}

	// 플레이어 역할 초기화 (호스트만)
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->HasAuthority())
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
			if (SessionSubsystem)
			{
				SessionSubsystem->ClearPlayerRoles();
			}
		}
	}

	// GameState 이벤트 바인딩
	UWorld* World = GetWorld();
	if (World)
	{
		ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>();
		if (LobbyGS)
		{
			LobbyGS->OnPlayerRolesChanged.AddDynamic(this, &ULobbyWidget::UpdatePlayerList);
			// 초기 업데이트
			UpdatePlayerList();
		}
	}
}

void ULobbyWidget::OnSwapRolesClicked()
{
	if (!bIsHost) return;

	// 역할 교체 로직 - MainMenuGameMode를 통해 처리
	UWorld* World = GetWorld();
	if (World)
	{
		AMainMenuGameMode* GameMode = Cast<AMainMenuGameMode>(World->GetAuthGameMode());
		if (GameMode)
		{
			GameMode->SwapPlayerRoles();
		}
	}
}

void ULobbyWidget::OnStartGameClicked()
{
	if (!bIsHost) return;

	// 게임 시작 로직
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->HasAuthority())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel("/Game/Levels/Test?listen");
		}
	}
}

void ULobbyWidget::OnBackClicked()
{
	if (!bIsHost) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
	if (!SessionSubsystem) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GameState = World->GetGameState();
	if (GameState)
	{
		// 클라이언트들 먼저 킥
		TArray<APlayerState*> PlayerArray = GameState->PlayerArray;
		for (APlayerState* PS : PlayerArray)
		{
			if (PS)
			{
				APlayerController* PC = Cast<APlayerController>(PS->GetOwner());
				if (PC && !PC->HasAuthority())
				{
					// 클라이언트 연결 끊기
					PC->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("Host closed the session")));
				}
			}
		}
	}

	// 세션 파괴
	SessionSubsystem->DestroyServer();

	// 위젯 숨기기 및 메인 메뉴로
	SetVisibility(ESlateVisibility::Hidden);

	// MainMenuController를 통해 메인 메뉴 표시
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->IsA<AMainMenuController>())
	{
		AMainMenuController* MainMenuController = Cast<AMainMenuController>(PC);
		if (MainMenuController)
		{
			MainMenuController->ShowMainMenuWidget();
		}
	}
}

void ULobbyWidget::UpdatePlayerList()
{
	UWorld* World = GetWorld();
	if (!World) return;

	ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>();
	if (!LobbyGS) return;

	// 세션 ID 업데이트
	if (SessionIdText && !LobbyGS->SessionId.IsEmpty())
	{
		SessionIdText->SetText(FText::FromString(FString::Printf(TEXT("Session ID: %s"), *LobbyGS->SessionId)));
		CurrentSessionId = LobbyGS->SessionId;
	}

	// GameState에서 리플리케이트된 역할 정보 사용
	const TArray<FPlayerRoleInfo>& RoleInfos = LobbyGS->PlayerRoles;

	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, FString::Printf(TEXT("UpdatePlayerList: %d players"), RoleInfos.Num()));

	// 역할별로 플레이어 찾기 및 표시
	bool bRole0Found = false;
	bool bRole1Found = false;

	for (const FPlayerRoleInfo& Info : RoleInfos)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, FString::Printf(TEXT("Player: %s, Role: %d"), *Info.PlayerName, Info.Role));

		if (Info.Role == 0 && Player1NameText)
		{
			Player1NameText->SetText(FText::FromString(FString::Printf(TEXT("1P (AttackCat): %s"), *Info.PlayerName)));
			bRole0Found = true;
		}
		else if (Info.Role == 1 && Player2NameText)
		{
			Player2NameText->SetText(FText::FromString(FString::Printf(TEXT("2P (BiteCat): %s"), *Info.PlayerName)));
			bRole1Found = true;
		}
	}

	// 빈 슬롯 표시
	if (!bRole0Found && Player1NameText)
	{
		Player1NameText->SetText(FText::FromString(TEXT("1P (AttackCat): Waiting...")));
	}
	if (!bRole1Found && Player2NameText)
	{
		Player2NameText->SetText(FText::FromString(TEXT("2P (BiteCat): Waiting...")));
	}
}

void ULobbyWidget::SetIsHost(bool bInIsHost)
{
	bIsHost = bInIsHost;

	// 버튼 활성화/비활성화 - 호스트만 활성화
	if (SwapRolesButton) SwapRolesButton->SetIsEnabled(bIsHost);
	if (StartGameButton) StartGameButton->SetIsEnabled(bIsHost);
	if (BackButton) BackButton->SetIsEnabled(bIsHost);
	if (CopySessionIdButton) CopySessionIdButton->SetIsEnabled(true); // 복사 버튼은 모두 활성화
}

void ULobbyWidget::UpdatePlayerRoles(const FString& Player1Name, const FString& Player2Name)
{
	if (Player1NameText)
	{
		Player1NameText->SetText(FText::FromString(FString::Printf(TEXT("1P: %s"), *Player1Name)));
	}

	if (Player2NameText)
	{
		Player2NameText->SetText(FText::FromString(FString::Printf(TEXT("2P: %s"), *Player2Name)));
	}
}

void ULobbyWidget::SetSessionId(const FString& SessionId)
{
	CurrentSessionId = SessionId;

	if (SessionIdText)
	{
		SessionIdText->SetText(FText::FromString(FString::Printf(TEXT("Session ID: %s"), *SessionId)));
	}
}

void ULobbyWidget::OnCopySessionIdClicked()
{
	if (!CurrentSessionId.IsEmpty())
	{
		FPlatformApplicationMisc::ClipboardCopy(*CurrentSessionId);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Session ID copied to clipboard!"));
	}
}

