// Fill out your copyright notice in the Description page of Project Settings.


#include "Widdget/MultiSessionWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/MultiSessionSubsystem.h"

void UMultiSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CreateServerButton)
	{
		CreateServerButton->OnClicked.AddDynamic(this, &UMultiSessionWidget::OnCreateServerClicked);
	}

	if (JoinServerButton)
	{
		JoinServerButton->OnClicked.AddDynamic(this, &UMultiSessionWidget::OnJoinServerClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UMultiSessionWidget::OnBackClicked);
	}
}

void UMultiSessionWidget::OnCreateServerClicked()
{
	UMultiSessionSubsystem* Server = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
	if (Server)
	{
		Server->CreateServer();
	}
}

void UMultiSessionWidget::OnJoinServerClicked()
{
	if (SessionIdTextBox)
	{
		FString SessionId = SessionIdTextBox->GetText().ToString();
		UMultiSessionSubsystem* Server = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
		if (Server)
		{
			Server->FindServers(SessionId);
		}
	}
}

void UMultiSessionWidget::OnBackClicked()
{
	OnBackButtonClicked.Broadcast();
}

