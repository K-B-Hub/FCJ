// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuController.h"
#include "Subsystem/MultiSessionSubsystem.h"
#include "FCJ/Widdget/MainMenuWidget.h"
#include "FCJ/Widdget/MultiSessionWidget.h"
#include "FCJ/Widdget/LobbyWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMainMenuController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	if (!IsLocalPlayerController())
	{
		return;
	}

	// 세션 상태 확인
	UMultiSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
	bool bInServer = SessionSubsystem && SessionSubsystem->bInServer;

	// 세션에 있으면 로비 표시, 없으면 메인 메뉴 표시
	if (bInServer)
	{
		FString SessionId = SessionSubsystem->GetCurrentSessionId();
		ShowLobbyWidget(SessionId);
	}
	else
	{
		ShowMainMenuWidget();
	}
}

void AMainMenuController::ShowMultiSessionWidget()
{
	HideAllWidgets();

	UMultiSessionWidget* Widget = GetOrCreateWidget(MultiSessionWidgetClass, MultiSessionWidget);
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		if (!Widget->OnBackButtonClicked.IsBound())
		{
			Widget->OnBackButtonClicked.AddDynamic(this, &AMainMenuController::ShowMainMenuWidget);
		}
	}
}

void AMainMenuController::ShowMainMenuWidget()
{
	HideAllWidgets();

	// 로비 위젯은 완전히 제거 (세션 종료)
	if (LobbyWidget)
	{
		LobbyWidget->RemoveFromParent();
		LobbyWidget = nullptr;
	}

	UMainMenuWidget* Widget = GetOrCreateWidget(MainMenuWidgetClass, MainMenuWidget);
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		if (!Widget->OnMultiPlayButtonClicked.IsBound())
		{
			Widget->OnMultiPlayButtonClicked.AddDynamic(this, &AMainMenuController::ShowMultiSessionWidget);
		}
	}
}

void AMainMenuController::ShowLobbyWidget(const FString& SessionId)
{
	HideAllWidgets();

	ULobbyWidget* Widget = GetOrCreateWidget(LobbyWidgetClass, LobbyWidget);
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		Widget->SetSessionId(SessionId);
		Widget->SetIsHost(HasAuthority());
	}
}

void AMainMenuController::ClientShowLobbyWidget_Implementation()
{
	UMultiSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
	FString SessionId = SessionSubsystem ? SessionSubsystem->GetCurrentSessionId() : TEXT("");

	ShowLobbyWidget(SessionId);

	if (LobbyWidget)
	{
		LobbyWidget->SetIsHost(false); // 클라이언트는 항상 false
	}
}

void AMainMenuController::ClientReturnToMainMenu_Implementation()
{
	// 세션 정보 정리
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
		if (SessionSubsystem)
		{
			// 세션 정리
			SessionSubsystem->LeaveSession();

			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Client session cleared, returning to main menu"));
		}
	}
	// 메인 메뉴로 복귀
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

// ========== 헬퍼 함수 ==========

void AMainMenuController::HideAllWidgets()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MultiSessionWidget)
	{
		MultiSessionWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (LobbyWidget)
	{
		LobbyWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

template<typename T>
T* AMainMenuController::GetOrCreateWidget(TSubclassOf<T> WidgetClass, T*& WidgetRef)
{
	if (!WidgetRef && WidgetClass)
	{
		WidgetRef = CreateWidget<T>(this, WidgetClass);
		if (WidgetRef)
		{
			WidgetRef->AddToViewport();
			WidgetRef->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	return WidgetRef;
}
