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

	// GameState 이벤트 바인딩
	UWorld* World = GetWorld();
	if (World)
	{
		ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>();
		if (LobbyGS)
		{
			UpdatePlayerRoles(LobbyGS->PlayerRoles.Num() > 0 ? LobbyGS->PlayerRoles[0].PlayerName : TEXT("Waiting..."),
				LobbyGS->PlayerRoles.Num() > 1 ? LobbyGS->PlayerRoles[1].PlayerName : TEXT("Waiting..."));
			
			LobbyGS->OnPlayerRolesChanged.AddDynamic(this, &ULobbyWidget::UpdatePlayerList);

			// 서버인 경우 또는 데이터가 이미 복제된 경우 즉시 업데이트
			if (GetOwningPlayer() && GetOwningPlayer()->HasAuthority())
			{
				UpdatePlayerList();
			}
			else if (!LobbyGS->SessionId.IsEmpty() || LobbyGS->PlayerRoles.Num() > 0)
			{
				// 클라이언트지만 이미 데이터가 복제된 경우
				UpdatePlayerList();
			}
			// 그 외의 경우는 OnRep 함수가 호출될 때 자동으로 업데이트됨
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
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
	if (!SessionSubsystem) return;

	if (bIsHost)
	{
		// 호스트: 세션 파괴 (클라이언트 강퇴는 DestroyServer 내부에서 처리)
		SessionSubsystem->DestroyServer();

		// 위젯 숨기기 및 메인 메뉴로
		SetVisibility(ESlateVisibility::Hidden);

		// MainMenuController를 통해 메인 메뉴 표시
		if (PC->IsA<AMainMenuController>())
		{
			AMainMenuController* MainMenuController = Cast<AMainMenuController>(PC);
			if (MainMenuController)
			{
				MainMenuController->ShowMainMenuWidget();
			}
		}
	}
	else
	{
		// 클라이언트: 세션에서 나가기
		SessionSubsystem->LeaveSession(); // 클라이언트도 자신의 세션 정보 정리

		// 메인 메뉴로 복귀
		UGameplayStatics::OpenLevel(PC, FName("MainMenu"));
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

	// 버튼 활성화/비활성화
	if (SwapRolesButton) SwapRolesButton->SetIsEnabled(bIsHost); // 호스트만
	if (StartGameButton) StartGameButton->SetIsEnabled(bIsHost); // 호스트만
	if (BackButton) BackButton->SetIsEnabled(true); // 모두 활성화
	if (CopySessionIdButton) CopySessionIdButton->SetIsEnabled(true); // 모두 활성화
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

