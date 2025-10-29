// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FCJGameInstance.generated.h"

/**
 * FCJ Game Instance - 레벨 전환 간에도 유지되는 게임 인스턴스
 * 로딩 위젯 등 레벨 전환 중에도 표시되어야 하는 UI를 관리
 */
UCLASS()
class FCJ_API UFCJGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	/**
	 * 레벨 로딩 위젯 표시
	 * 레벨 전환 중에도 화면에 유지됨
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLevelLoadingWidget();

	/**
	 * 레벨 로딩 위젯 숨김
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideLevelLoadingWidget();

	/**
	 * 스트리밍 완료 체크 시작
	 * 레벨이 로드된 후 호출하여 모든 리소스 스트리밍이 완료될 때까지 대기
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void StartCheckingStreamingCompletion();

private:
	/**
	 * 스트리밍 완료 체크 타이머 콜백
	 */
	void CheckStreamingCompletion();

	// 스트리밍 체크 타이머 핸들
	FTimerHandle StreamingCheckTimerHandle;
	// 로딩 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class ULoadingWidget> LoadingWidgetClass;

	// 로딩 위젯 인스턴스 (레벨 전환 간에도 유지됨)
	UPROPERTY()
	class ULoadingWidget* LoadingWidget;
};