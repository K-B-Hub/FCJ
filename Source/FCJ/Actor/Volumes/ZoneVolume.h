// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "ZoneVolume.generated.h"

class AClearTrigger;

// Zone 클리어 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZoneCleared);

/**
 * 게임 내 구역을 의미하는 볼륨
 * bIsPuzzleZone이 true이면 구역 내 모든 ClearTrigger를 추적하며,
 * 모든 ClearTrigger가 클리어되면 Zone도 클리어됩니다.
 */
UCLASS()
class FCJ_API AZoneVolume : public AActor
{
	GENERATED_BODY()

public:
	AZoneVolume();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 볼륨 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UBoxComponent* VolumeBox;

	// 퍼즐 구역 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings", meta = (ToolTip = "true면 구역 내 ClearTrigger를 추적하여 퍼즐 클리어 여부를 판단합니다"))
	bool bIsPuzzleZone;

	// 구역 번호 (낮은 번호일수록 이전 구역)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings", meta = (ToolTip = "구역 번호 - 캐릭터 간 거리가 멀어질 때 더 낮은 번호의 구역으로 순간이동합니다"))
	int32 ZoneNumber;

	// Zone 클리어 여부 (모든 ClearTrigger가 클리어되면 true)
	// 리플리케이션을 통해 서버-클라이언트 간 동기화
	UPROPERTY(ReplicatedUsing = OnRep_IsZoneCleared, VisibleAnywhere, BlueprintReadOnly, Category = "Zone State")
	bool bIsZoneCleared;

	// bIsZoneCleared 리플리케이션 콜백
	UFUNCTION()
	void OnRep_IsZoneCleared();

	// 구역 내 ClearTrigger 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone State")
	TArray<AClearTrigger*> ClearTriggers;

private:
	/**
	 * ClearTrigger의 클리어 상태 변경 시 호출되는 콜백 함수
	 * @param bIsCleared 클리어 상태
	 */
	UFUNCTION()
	void OnClearTriggerStateChanged(bool bIsCleared);

	/**
	 * 모든 ClearTrigger가 클리어되었는지 확인합니다.
	 */
	void CheckAllTriggersCleared();

	/**
	 * 볼륨과 오버랩되는 모든 ClearTrigger를 수집합니다.
	 */
	void CollectClearTriggers();

public:
	// Zone 클리어 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Zone")
	FOnZoneCleared OnZoneCleared;

	/**
	 * Zone이 클리어되었는지 확인합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone")
	bool IsZoneCleared() const { return bIsZoneCleared; }

	/**
	 * 퍼즐 구역인지 확인합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone")
	bool IsPuzzleZone() const { return bIsPuzzleZone; }

	/**
	 * 구역 내 ClearTrigger 개수를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone")
	int32 GetClearTriggerCount() const { return ClearTriggers.Num(); }

	/**
	 * 구역 내 클리어된 ClearTrigger 개수를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone")
	int32 GetClearedTriggerCount() const;

	/**
	 * 구역 번호를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone")
	int32 GetZoneNumber() const { return ZoneNumber; }
	
	UFUNCTION(BlueprintCallable, Category = "Zone")
	void SetZoneNumber(int32 zoneNum) { ZoneNumber = zoneNum; }

	/**
	 * VolumeBox를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Zone")
	UBoxComponent* GetVolumeBox() const { return VolumeBox; }
};
