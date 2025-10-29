// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/FCJGameInstance.h"
#include "Widdget/LoadingWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StreamableManager.h"
#include "TimerManager.h"

void UFCJGameInstance::Init()
{
	Super::Init();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("🟢 FCJGameInstance::Init()"));
}

void UFCJGameInstance::Shutdown()
{
	// 타이머 정리
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(StreamingCheckTimerHandle);
	}

	// 위젯 정리
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("🔴 FCJGameInstance::Shutdown()"));

	Super::Shutdown();
}

void UFCJGameInstance::ShowLevelLoadingWidget()
{
	if (!LoadingWidgetClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("⚠️ LoadingWidgetClass not set in FCJGameInstance!"));
		return;
	}

	// 위젯이 없으면 생성
	if (!LoadingWidget)
	{
		// GameInstance에서 위젯을 생성할 때는 World의 첫 번째 LocalPlayer를 사용
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			if (PC)
			{
				LoadingWidget = CreateWidget<ULoadingWidget>(PC, LoadingWidgetClass);
			}
		}
	}

	// 위젯 표시
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget->AddToViewport(1000); // 최상위 Z-order
		LoadingWidget->SetVisibility(ESlateVisibility::Visible);
		LoadingWidget->ShowLevelLoad();

		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("✅ Level loading widget shown (GameInstance)"));
	}
}

void UFCJGameInstance::HideLevelLoadingWidget()
{
	// 타이머 정리
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(StreamingCheckTimerHandle);
	}

	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;

		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("✅ Level loading widget hidden (GameInstance)"));
	}
}

void UFCJGameInstance::StartCheckingStreamingCompletion()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("⚠️ World is null, cannot check streaming"));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("🔄 Started checking streaming completion..."));

	// 0.1초마다 스트리밍 상태 체크
	World->GetTimerManager().SetTimer(StreamingCheckTimerHandle, this, &UFCJGameInstance::CheckStreamingCompletion, 0.1f, true);
}

void UFCJGameInstance::CheckStreamingCompletion()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// FStreamingManager로 스트리밍 상태 확인
	if (IStreamingManager::Get().GetNumWantingResources() == 0)
	{
		// 모든 스트리밍 완료
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("✅ Streaming completed! Hiding loading widget..."));

		// 타이머 정지
		World->GetTimerManager().ClearTimer(StreamingCheckTimerHandle);

		// 로딩 위젯 숨김
		HideLevelLoadingWidget();
	}
	else
	{
		// 여전히 스트리밍 중
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, TEXT("⏳ Still streaming..."));
	}
}