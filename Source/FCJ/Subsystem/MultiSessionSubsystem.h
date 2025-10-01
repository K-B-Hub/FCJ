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

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	// 현재 세션 ID 가져오기
	UFUNCTION(BlueprintCallable)
	FString GetCurrentSessionId() const;

	// 플레이어 역할 저장 (UniqueNetId를 문자열로 저장)
	UFUNCTION(BlueprintCallable)
	void SetPlayerRole(const FString& PlayerNetId, int32 Role);

	// 플레이어 역할 가져오기
	UFUNCTION(BlueprintCallable)
	int32 GetPlayerRole(const FString& PlayerNetId) const;

	// 역할 교체
	UFUNCTION(BlueprintCallable)
	void SwapPlayerRoles();

	// 모든 플레이어 역할 초기화
	UFUNCTION(BlueprintCallable)
	void ClearPlayerRoles();

private:
	FName serverName;

	// 플레이어 역할 매핑 (UniqueNetId String -> Role Index)
	// 0 = 1P (HoldingCat), 1 = 2P (BiteCat)
	TMap<FString, int32> PlayerRoles;

	// 찾고자 하는 세션 ID
	FString TargetSessionId;

	TSharedPtr<FOnlineSessionSearch> sessionSearch;
};
