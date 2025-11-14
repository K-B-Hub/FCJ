// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ChildActorComponent.h"
#include "Components/BoxComponent.h"
#include "ClearTrigger.generated.h"

class ABaseTrigger;

// 클리어 상태 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClearStateChanged, bool, bIsCleared);

/**
 * 클리어 트리거 시스템
 * BaseTrigger를 자식 액터 컴포넌트로 가지며,
 * BaseTrigger의 상태에 따라 클리어 상태가 동적으로 변경됩니다.
 * 트리거가 활성화되면 true, 비활성화되면 false가 됩니다.
 */
UCLASS()
class FCJ_API AClearTrigger : public AActor
{
	GENERATED_BODY()

public:
	AClearTrigger();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 콜리전 박스 (ZoneVolume과 오버랩 감지용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UBoxComponent* CollisionBox;

	// 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* TriggerComponent;

	// 클리어 여부 (트리거 상태에 따라 변경 가능)
	// 리플리케이션을 통해 서버-클라이언트 간 동기화
	UPROPERTY(ReplicatedUsing = OnRep_IsCleared, VisibleAnywhere, BlueprintReadOnly, Category = "Clear State")
	bool bIsCleared;

	// bIsCleared 리플리케이션 콜백
	UFUNCTION()
	void OnRep_IsCleared();

private:
	// 캐시된 트리거 액터
	UPROPERTY()
	ABaseTrigger* CachedTrigger;

	/**
	 * 트리거 상태 변경 시 호출되는 콜백 함수
	 * @param bNewState 새로운 트리거 상태
	 */
	UFUNCTION()
	void OnTriggerStateChangedCallback(bool bNewState);

public:
	// 클리어 상태 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Clear")
	FOnClearStateChanged OnClearStateChanged;

	/**
	 * Trigger 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Clear")
	ABaseTrigger* GetTrigger() const { return CachedTrigger; }

	/**
	 * 클리어 여부를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Clear")
	bool IsCleared() const { return bIsCleared; }
};
