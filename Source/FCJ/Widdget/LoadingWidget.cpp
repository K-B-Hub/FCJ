// Fill out your copyright notice in the Description page of Project Settings.


#include "Widdget/LoadingWidget.h"
#include "Components/Widget.h"

void ULoadingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기화 시 모든 위젯 숨김
	HideAllWidgets();

	// SessionFailButton 클릭 시 모든 위젯 숨김
	if (SessionFailButton)
	{
		// 중복 바인딩 방지: 이미 바인딩되어 있으면 제거 후 다시 바인딩
		SessionFailButton->OnClicked.RemoveDynamic(this, &ULoadingWidget::HideAllWidgets);
		SessionFailButton->OnClicked.AddDynamic(this, &ULoadingWidget::HideAllWidgets);
	}
}

void ULoadingWidget::ShowSessionLoad()
{
	HideAllWidgets();

	if (SessionLoadWidget)
	{
		SessionLoadWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULoadingWidget::ShowSessionFail()
{
	HideAllWidgets();

	if (SessionFailContainer)
	{
		SessionFailContainer->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULoadingWidget::ShowLevelLoad()
{
	HideAllWidgets();

	if (LevelLoadWidget)
	{
		LevelLoadWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ULoadingWidget::HideAllWidgets()
{
	if (SessionLoadWidget)
	{
		SessionLoadWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SessionFailContainer)
	{
		SessionFailContainer->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (LevelLoadWidget)
	{
		LevelLoadWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
