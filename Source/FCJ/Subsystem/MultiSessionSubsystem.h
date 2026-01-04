// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiSessionSubsystem.generated.h"

class FOnlineSessionSearch;
/**
 * 
 */
UCLASS()
class FCJ_API UMultiSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiSessionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	IOnlineSessionPtr sessionInterface;

	UFUNCTION()
	void CreateServer();
	UFUNCTION()
	void FindServers(FString SessionId);
	UFUNCTION()
	void DestroyServer();
	UFUNCTION()
	void LeaveSession();

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnLeaveSessionComplete(FName SessionName, bool bWasSuccessful);

	// 현재 세션 ID 가져오기
	UFUNCTION(BlueprintCallable)
	FString GetCurrentSessionId() const;

	// 플레이어 역할 정보 관리 (레벨 전환 시에도 유지됨)
	UFUNCTION(BlueprintCallable)
	void SetPlayerRole(const FString& NetId, int32 Role);

	UFUNCTION(BlueprintCallable)
	int32 GetPlayerRole(const FString& NetId) const;

	UFUNCTION(BlueprintCallable)
	void ClearPlayerRoles();

	bool bInServer = false;

private:
	// 플레이어 역할 정보 저장 (NetId -> Role: 0 = HybridCat (AttackCat 외형), 1 = HybridCat (BiteCat 외형))
	TMap<FString, int32> PlayerRoles;
	FName serverName;

	// 찾고자 하는 세션 ID
	FString TargetSessionId;

	TSharedPtr<FOnlineSessionSearch> sessionSearch;

	// 세션 재생성 플래그
	bool bPendingCreateServer = false;
};
