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
	FString PlayerNetId;

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	int32 Role; // 0 = AttackCat, 1 = BiteCat

	FPlayerRoleInfo()
		: PlayerNetId(TEXT("")), PlayerName(TEXT("")), Role(-1)
	{
	}

	FPlayerRoleInfo(const FString& InNetId, const FString& InName, int32 InRole)
		: PlayerNetId(InNetId), PlayerName(InName), Role(InRole)
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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 플레이어 역할 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPlayerRolesChanged OnPlayerRolesChanged;

	// 플레이어 역할 배열 (리플리케이트됨)
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRoles, BlueprintReadOnly, Category = "Lobby")
	TArray<FPlayerRoleInfo> PlayerRoles;

	// 세션 ID (리플리케이트됨)
	UPROPERTY(ReplicatedUsing = OnRep_SessionId, BlueprintReadOnly, Category = "Lobby")
	FString SessionId;

	// 역할 업데이트 (서버에서만 호출)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdatePlayerRoles(const TArray<FPlayerRoleInfo>& NewRoles);

	// 플레이어 추가 (서버에서만 호출)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void AddPlayer(const FString& PlayerNetId, const FString& PlayerName);

	// 플레이어 제거 (서버에서만 호출)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void RemovePlayer(const FString& PlayerNetId);

	// 역할 교체 (서버에서만 호출)
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SwapPlayerRoles();

	// 플레이어 역할 가져오기
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	int32 GetPlayerRole(const FString& PlayerNetId) const;

	// RepNotify 함수
	UFUNCTION()
	void OnRep_PlayerRoles();

	UFUNCTION()
	void OnRep_SessionId();

private:
	// 모든 역할 정보를 MultiSessionSubsystem에 동기화
	void SyncRolesToSubsystem();
};
