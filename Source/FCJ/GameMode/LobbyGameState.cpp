// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameState.h"
#include "Net/UnrealNetwork.h"

ALobbyGameState::ALobbyGameState()
{
	bReplicates = true;
}

void ALobbyGameState::BeginPlay()
{
	Super::BeginPlay();

	// 디버그: 생성 시점 추적
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
		TEXT("🟢 LobbyGameState::BeginPlay() - GameState Created"));
}

void ALobbyGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 디버그: 파괴 시점 추적
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		FString::Printf(TEXT("🔴 LobbyGameState::EndPlay() - Reason: %d"), static_cast<int32>(EndPlayReason)));

	Super::EndPlay(EndPlayReason);
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

void ALobbyGameState::OnRep_SessionId()
{
	// 세션 ID가 복제되었을 때 클라이언트에서 처리
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("SessionId replicated: %s"), *SessionId));

	// 세션 ID가 복제되면 UI 업데이트 알림
	OnPlayerRolesChanged.Broadcast();
}

void ALobbyGameState::AddPlayer(const FString& PlayerNetId, const FString& PlayerName)
{
	if (!HasAuthority()) return;

	// 이미 존재하는 플레이어인지 확인 (NetId로)
	for (const FPlayerRoleInfo& Info : PlayerRoles)
	{
		if (Info.PlayerNetId == PlayerNetId)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Player %s (NetId: %s) already exists"), *PlayerName, *PlayerNetId));
			return;
		}
	}

	// 새 역할 할당 (0, 1 순서대로)
	int32 NewRole = PlayerRoles.Num();
	if (NewRole >= 2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Lobby is full!"));
		return;
	}

	FPlayerRoleInfo NewPlayer(PlayerNetId, PlayerName, NewRole);
	PlayerRoles.Add(NewPlayer);

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
		FString::Printf(TEXT("Added player: %s (NetId: %s) with role %d"), *PlayerName, *PlayerNetId, NewRole));

	// 서버에서도 이벤트 브로드캐스트
	OnPlayerRolesChanged.Broadcast();
}

void ALobbyGameState::RemovePlayer(const FString& PlayerNetId)
{
	if (!HasAuthority()) return;

	// PlayerNetId로 찾아서 제거
	int32 RemovedIndex = -1;
	for (int32 i = 0; i < PlayerRoles.Num(); ++i)
	{
		if (PlayerRoles[i].PlayerNetId == PlayerNetId)
		{
			RemovedIndex = i;
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Removing player: %s (NetId: %s)"), *PlayerRoles[i].PlayerName, *PlayerNetId));
			break;
		}
	}

	if (RemovedIndex != -1)
	{
		PlayerRoles.RemoveAt(RemovedIndex);

		// 역할 재할당 (남은 플레이어들에게 0부터 순서대로)
		for (int32 i = 0; i < PlayerRoles.Num(); ++i)
		{
			PlayerRoles[i].Role = i;
		}

		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
			FString::Printf(TEXT("Player removed. Remaining players: %d"), PlayerRoles.Num()));

		OnPlayerRolesChanged.Broadcast();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
			FString::Printf(TEXT("Player with NetId %s not found"), *PlayerNetId));
	}
}

void ALobbyGameState::SwapPlayerRoles()
{
	if (!HasAuthority()) return;

	if (PlayerRoles.Num() < 2)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Need at least 2 players to swap roles"));
		return;
	}

	// 두 플레이어의 역할 교체
	int32 TempRole = PlayerRoles[0].Role;
	PlayerRoles[0].Role = PlayerRoles[1].Role;
	PlayerRoles[1].Role = TempRole;

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
		FString::Printf(TEXT("Swapped roles: %s (%d) <-> %s (%d)"),
			*PlayerRoles[0].PlayerName, PlayerRoles[0].Role,
			*PlayerRoles[1].PlayerName, PlayerRoles[1].Role));

	// 서버에서도 이벤트 브로드캐스트
	OnPlayerRolesChanged.Broadcast();
}

int32 ALobbyGameState::GetPlayerRole(const FString& PlayerNetId) const
{
	// PlayerNetId로 역할 찾기
	for (const FPlayerRoleInfo& Info : PlayerRoles)
	{
		if (Info.PlayerNetId == PlayerNetId)
		{
			return Info.Role;
		}
	}

	// 찾지 못한 경우 -1 반환
	return -1;
}
