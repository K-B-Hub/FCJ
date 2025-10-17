// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuController.generated.h"

/**
 * 
 */
UCLASS()
class FCJ_API AMainMenuController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class UMultiSessionWidget> MultiSessionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class ULobbyWidget> LobbyWidgetClass;

	UPROPERTY()
	class UMainMenuWidget* MainMenuWidget;

	UPROPERTY()
	class UMultiSessionWidget* MultiSessionWidget;

	UPROPERTY()
	class ULobbyWidget* LobbyWidget;

public:
	UFUNCTION()
	void ShowMultiSessionWidget();

	UFUNCTION()
	void ShowMainMenuWidget();

	UFUNCTION(BlueprintCallable)
	void ShowLobbyWidget(const FString& SessionId);

	// 클라이언트 RPC - 로비 위젯 표시
	UFUNCTION(Client, Reliable)
	void ClientShowLobbyWidget();

	// 서버에 의해 강퇴될 때 호출됨
	UFUNCTION(Client, Reliable)
	void ClientReturnToMainMenu();

private:
	// 헬퍼 함수: 모든 위젯 숨기기
	void HideAllWidgets();

	// 헬퍼 함수: 위젯 생성 또는 가져오기
	template<typename T>
	T* GetOrCreateWidget(TSubclassOf<T> WidgetClass, T*& WidgetRef);
};
