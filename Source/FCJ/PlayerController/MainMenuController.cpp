// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuController.h"

#include "../../../../../UE_5.6/Engine/Source/Runtime/UMG/Public/Components/Image.h"
#include "FCJ/Widdget/MainMenuWidget.h"
#include "FCJ/Widdget/MultiSessionWidget.h"
#include "FCJ/Widdget/LobbyWidget.h"
#include "Blueprint/UserWidget.h"

void AMainMenuController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	if (IsLocalPlayerController())
	{
		UWorld* World = GetWorld();
		bool bIsInMultiplayerSession = World && World->GetNetMode() != NM_Standalone;

		// 멀티플레이 세션이 아닐 때만 메인 메뉴와 세션 위젯 표시
		if (!bIsInMultiplayerSession)
		{
			if (MainMenuWidgetClass)
			{
				MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
				if (MainMenuWidget)
				{
					MainMenuWidget->AddToViewport();
					MainMenuWidget->OnMultiPlayButtonClicked.AddDynamic(this, &AMainMenuController::ShowMultiSessionWidget);
				}
			}

			if (MultiSessionWidgetClass)
			{
				MultiSessionWidget = CreateWidget<UMultiSessionWidget>(this, MultiSessionWidgetClass);
				if (MultiSessionWidget)
				{
					MultiSessionWidget->AddToViewport();
					MultiSessionWidget->SetVisibility(ESlateVisibility::Hidden);
					MultiSessionWidget->OnBackButtonClicked.AddDynamic(this, &AMainMenuController::ShowMainMenuWidget);
				}
			}
		}

		// LobbyWidget은 필요할 때 ShowLobbyWidget에서 생성
		// BeginPlay에서는 생성하지 않음
	}
}

void AMainMenuController::ShowMultiSessionWidget()
{
	if (MainMenuWidget && MultiSessionWidget)
	{
		MainMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		MultiSessionWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainMenuController::ShowMainMenuWidget()
{
	if (MainMenuWidget && MultiSessionWidget)
	{
		MultiSessionWidget->SetVisibility(ESlateVisibility::Hidden);
		MainMenuWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainMenuController::ShowLobbyWidget(const FString& SessionId)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
		FString::Printf(TEXT("ShowLobbyWidget called: LobbyWidget=%s, SessionId=%s, HasAuthority=%d, IsLocal=%d"),
		LobbyWidget ? TEXT("Valid") : TEXT("NULL"), *SessionId, HasAuthority(), IsLocalController()));

	if (!LobbyWidget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LobbyWidget is NULL! Creating now..."));
		if (LobbyWidgetClass)
		{
			LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
			if (LobbyWidget)
			{
				LobbyWidget->AddToViewport();
			}
		}
	}

	if (LobbyWidget)
	{
		// 다른 위젯 숨기기
		if (MainMenuWidget)
		{
			MainMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		if (MultiSessionWidget)
		{
			MultiSessionWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		// 로비 위젯 표시 및 세션 ID 설정
		LobbyWidget->SetVisibility(ESlateVisibility::Visible);
		LobbyWidget->SetSessionId(SessionId);
		LobbyWidget->SetIsHost(HasAuthority());

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("LobbyWidget shown successfully!"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create LobbyWidget!"));
	}
}

void AMainMenuController::ClientShowLobbyWidget_Implementation()
{
	if (LobbyWidget)
	{
		// 다른 위젯 숨기기
		if (MainMenuWidget)
		{
			MainMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		if (MultiSessionWidget)
		{
			MultiSessionWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		// 로비 위젯 표시
		LobbyWidget->SetVisibility(ESlateVisibility::Visible);
		LobbyWidget->SetIsHost(false); // 클라이언트는 항상 false
	}
}
