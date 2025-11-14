// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTrigger.generated.h"

// 트리거 상태 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerStateChanged, bool, bNewState);

/**
 * 베이스 트리거 클래스
 * 파생 클래스에서 다양한 조건으로 True/False 상태를 구현할 수 있습니다.
 * 포탑 제어, 문 개폐, 이벤트 트리거 등 다양한 용도로 사용 가능합니다.
 */
UCLASS(Abstract, Blueprintable)
class FCJ_API ABaseTrigger : public AActor
{
	GENERATED_BODY()

public:
	ABaseTrigger();

protected:
	virtual void BeginPlay() override;

	// 트리거의 현재 상태 (활성화/비활성화)
	// 리플리케이션을 통해 서버-클라이언트 간 동기화
	UPROPERTY(ReplicatedUsing = OnRep_IsActive, VisibleAnywhere, BlueprintReadOnly, Category = "Trigger State")
	bool bIsActive;

	// bIsActive 리플리케이션 콜백
	UFUNCTION()
	void OnRep_IsActive();

	// 트리거 상태를 반전할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings", meta = (ToolTip = "true면 트리거 상태를 반전하여 반환합니다"))
	bool bInvertTrigger = false;

public:
	virtual void Tick(float DeltaTime) override;

	// 트리거 상태 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Trigger")
	FOnTriggerStateChanged OnTriggerStateChanged;

	/**
	 * 트리거가 활성화되어 있는지 확인합니다.
	 * 파생 클래스에서 이 함수를 오버라이드하여 다양한 조건을 구현할 수 있습니다.
	 * bInvertTrigger가 true면 반전된 값을 반환합니다.
	 * @return 트리거가 활성화되어 있으면 true, 그렇지 않으면 false
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Trigger")
	bool IsTriggerActive() const;
	virtual bool IsTriggerActive_Implementation() const;

	/**
	 * 내부 트리거 상태를 반환합니다 (반전 적용 안됨).
	 * 파생 클래스에서 오버라이드하여 실제 트리거 로직을 구현합니다.
	 * @return 내부 트리거 상태
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Trigger")
	bool GetInternalTriggerState() const;
	virtual bool GetInternalTriggerState_Implementation() const;

	/**
	 * 트리거 상태를 직접 설정합니다.
	 * @param bNewActive 새로운 활성화 상태
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void SetTriggerActive(bool bNewActive);
};
