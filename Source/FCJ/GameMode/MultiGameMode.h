// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FCJ_API AMultiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMultiGameMode();

protected:
	// Blueprint configurable player controller class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class APlayerController> MultiPlayerControllerClass;

	// Blueprint configurable default pawn class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class APawn> DefaultPawnClass_Multi;

protected:
	virtual void BeginPlay() override;

public:
	// Override to set custom classes
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable)
	void HostLANGame();

	UFUNCTION(BlueprintCallable)
	void JoinLANGame();

	// 플레이어 역할 교체
	UFUNCTION(BlueprintCallable)
	void SwapPlayerRoles();

	// 플레이어 역할 가져오기 (0 = Player1, 1 = Player2)
	UFUNCTION(BlueprintCallable)
	int32 GetPlayerRole(APlayerController* PC) const;

protected:
	// 캐릭터 클래스들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Classes")
	TSubclassOf<class AHybridCat> AttackCatClass;  // 외형: AttackCat 스타일

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Classes")
	TSubclassOf<class AHybridCat> BiteCatClass;   // 외형: BiteCat 스타일

	// 캐릭터 간 최대 허용 거리 (이 거리를 초과하면 순간이동 발동)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings", meta = (ToolTip = "두 캐릭터 간의 최대 허용 거리 - 이 거리를 초과하면 더 낮은 구역의 캐릭터 쪽으로 순간이동합니다"))
	float MaxAllowedDistance = 3000.0f;

	// 거리 체크 주기 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Settings", meta = (ToolTip = "거리 체크를 수행하는 주기(초)"))
	float DistanceCheckInterval = 1.0f;

private:
	// Apply saved display settings
	void ApplySavedDisplaySettings();

	// 플레이어 역할 매핑 (PlayerController -> Role Index)
	// 0 = 1P (AttackCat), 1 = 2P (BiteCat)
	TMap<APlayerController*, int32> PlayerRoles;

	// 플레이어가 접속할 때 역할 할당 및 캐릭터 생성
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Subsystem에서 저장된 역할 정보 가져오기
	int32 GetRoleFromSubsystem(APlayerController* PC);

	// 역할에 맞는 캐릭터 스폰
	void SpawnCharacterForRole(APlayerController* PC, int32 role);

	// 거리 체크 타이머 핸들
	FTimerHandle DistanceCheckTimerHandle;

	/**
	 * 두 캐릭터 간의 거리를 체크하고 필요시 순간이동을 수행합니다.
	 */
	void CheckCharacterDistance();

	/**
	 * 캐릭터가 현재 속해 있는 ZoneVolume을 반환합니다.
	 * @param Character 확인할 캐릭터
	 * @return 캐릭터가 속한 ZoneVolume (없으면 nullptr)
	 */
	class AZoneVolume* GetCharacterZone(class ACatBase* Character);

	/**
	 * 캐릭터를 목표 캐릭터 위치로 순간이동시킵니다.
	 * @param CharacterToTeleport 순간이동할 캐릭터
	 * @param TargetCharacter 목표 캐릭터 (이 캐릭터 근처로 이동)
	 */
	void TeleportCharacter(ACatBase* CharacterToTeleport, ACatBase* TargetCharacter);
}; 
