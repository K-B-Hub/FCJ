// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameState.h"
#include "Net/UnrealNetwork.h"

ALobbyGameState::ALobbyGameState()
{
	bReplicates = true;
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, PlayerRoles);
	DOREPLIFETIME(ALobbyGameState, SessionId);
}

void ALobbyGameState::UpdatePlayerRoles(const TArray<FPlayerRoleInfo>& NewRoles)
{
	if (HasAuthority())
	{
		PlayerRoles = NewRoles;
		// 서버에서도 이벤트 브로드캐스트
		OnPlayerRolesChanged.Broadcast();
	}
}

void ALobbyGameState::OnRep_PlayerRoles()
{
	// 역할이 변경되었을 때 클라이언트에서 처리
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("Player roles updated!"));

	// 모든 LobbyWidget에게 업데이트 알림
	OnPlayerRolesChanged.Broadcast();
}
