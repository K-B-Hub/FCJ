// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../../../../UE_5.6/Engine/Source/Runtime/UMG/Public/Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

/**
 * 로딩 상태를 표시하는 위젯
 * SessionLoadWidget, SessionFailWidget, LevelLoadWidget 중 하나를 상황에 따라 표시
 */
UCLASS()
class FCJ_API ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	/**
	 * 세션 로딩 위젯 - 블루프린트에서 바인딩 필요
	 */
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SessionLoadWidget;

	/**
	 * 세션 실패 컨테이너 - SessionFailWidget과 SessionFailButton을 함께 담는 위젯
	 * 블루프린트에서 바인딩 필요
	 */
	UPROPERTY(meta = (BindWidget))
	UWidget* SessionFailContainer;

	/**
	 * 레벨 로딩 위젯 - 블루프린트에서 바인딩 필요
	 */
	UPROPERTY(meta = (BindWidget))
	UUserWidget* LevelLoadWidget;

	/**
	 * 세션 실패 버튼 - 이벤트 바인딩용
	 * SessionFailContainer 내부에 위치
	 */
	UPROPERTY(meta = (BindWidget))
	UButton* SessionFailButton;

	/**
	 * 세션 로딩 위젯을 표시
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowSessionLoad();

	/**
	 * 세션 실패 위젯을 표시
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowSessionFail();

	/**
	 * 레벨 로딩 위젯을 표시
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowLevelLoad();

private:
	/**
	 * 모든 위젯을 숨김 처리
	 */
	UFUNCTION()
	void HideAllWidgets();
};
