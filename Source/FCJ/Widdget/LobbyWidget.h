// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

/**
 *
 */
UCLASS()
class FCJ_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 플레이어 목록을 표시할 컨테이너
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PlayerListBox;

	// 1P 플레이어 이름 표시
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player1NameText;

	// 2P 플레이어 이름 표시
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Player2NameText;

	// 역할 교체 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* SwapRolesButton;

	// 게임 시작 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* StartGameButton;

	// 뒤로가기 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	// 세션 ID 표시 텍스트
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SessionIdText;

	// 세션 ID 복사 버튼
	UPROPERTY(meta = (BindWidget))
	UButton* CopySessionIdButton;

private:
	UFUNCTION()
	void OnSwapRolesClicked();

	UFUNCTION()
	void OnStartGameClicked();

	UFUNCTION()
	void OnBackClicked();

	UFUNCTION()
	void OnCopySessionIdClicked();

	// 플레이어 목록 업데이트
	UFUNCTION()
	void UpdatePlayerList();

	// 호스트 여부
	bool bIsHost;

	// 현재 세션 ID
	FString CurrentSessionId;

public:
	// 호스트 여부 설정
	void SetIsHost(bool bInIsHost);

	// 플레이어 역할 업데이트 (리플리케이션용)
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerRoles(const FString& Player1Name, const FString& Player2Name);

	// 세션 ID 설정
	UFUNCTION(BlueprintCallable)
	void SetSessionId(const FString& SessionId);
};
