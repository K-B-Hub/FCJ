// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

USTRUCT(BlueprintType)
struct FPlayerRoleInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Role; // 0 = AttackCat, 1 = BiteCat

	FPlayerRoleInfo()
		: PlayerName(TEXT("")), Role(-1)
	{
	}

	FPlayerRoleInfo(const FString& InName, int32 InRole)
		: PlayerName(InName), Role(InRole)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerRolesChanged);

/**
 * 로비 GameState - 플레이어 역할 정보를 리플리케이트
 */
UCLASS()
class FCJ_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALobbyGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어 역할 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPlayerRolesChanged OnPlayerRolesChanged;

	// 플레이어 역할 배열 (리플리케이트됨)
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRoles, BlueprintReadOnly, Category = "Lobby")
	TArray<FPlayerRoleInfo> PlayerRoles;

	// 세션 ID (리플리케이트됨)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
	FString SessionId;

	// 역할 업데이트 (서버에서만 호출)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdatePlayerRoles(const TArray<FPlayerRoleInfo>& NewRoles);

	// RepNotify 함수
	UFUNCTION()
	void OnRep_PlayerRoles();
};
