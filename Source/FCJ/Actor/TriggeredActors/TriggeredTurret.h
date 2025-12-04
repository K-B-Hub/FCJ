// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ChildActorComponent.h"
#include "Components/BoxComponent.h"
#include "TriggeredTurret.generated.h"

class ATurret;
class ABaseTrigger;

/**
 * 트리거에 의해 제어되는 포탑 시스템
 * Turret을 자식 액터 컴포넌트로 가지며, TriggerDetectionBox 위치에 겹치는 모든 BaseTrigger들을 감지합니다.
 * 감지된 모든 트리거가 활성화되어야만 Turret이 작동합니다.
 */
UCLASS()
class FCJ_API ATriggeredTurret : public AActor
{
	GENERATED_BODY()

public:
	ATriggeredTurret();

protected:
	virtual void BeginPlay() override;

	// 루트 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components)
	USceneComponent* RootSceneComponent;

	// 포탑 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "포탑 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* TurretComponent;

	// 트리거 감지용 박스 컴포넌트 (Blueprint에서 위치/크기 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "트리거 감지용 박스 - 이 박스와 겹치는 모든 BaseTrigger를 감지합니다"))
	UBoxComponent* TriggerDetectionBox;

	// 트리거 상태를 자동으로 체크할지 여부
	// 델리게이트 기반 이벤트 처리를 사용하지만, 수동으로 초기 상태를 체크할지 여부를 제어합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings", meta = (ToolTip = "BeginPlay에서 초기 트리거 상태를 체크할지 여부"))
	bool bAutoCheckTrigger = true;

private:
	// 캐시된 Turret 액터
	UPROPERTY()
	ATurret* CachedTurret;

	// TriggerDetectionBox와 겹치는 모든 BaseTrigger들
	UPROPERTY()
	TArray<ABaseTrigger*> ConnectedTriggers;

	/**
	 * 트리거 상태 변경 시 호출되는 콜백 함수
	 * @param bNewState 새로운 트리거 상태
	 */
	UFUNCTION()
	void OnTriggerStateChangedCallback(bool bNewState);

	/**
	 * 모든 연결된 트리거가 활성화되어 있는지 확인
	 * @return 모든 트리거가 활성화되어 있으면 true, 하나라도 비활성화되어 있으면 false
	 */
	bool CheckAllTriggersActive() const;

public:
	/**
	 * 트리거 상태를 수동으로 체크하고 포탑 상태를 업데이트합니다.
	 * 주로 초기화 시 사용되며, 이후에는 델리게이트가 자동으로 처리합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void CheckTriggerState();

	/**
	 * Turret 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	ATurret* GetTurret() const { return CachedTurret; }

	/**
	 * 연결된 모든 트리거를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	const TArray<ABaseTrigger*>& GetConnectedTriggers() const { return ConnectedTriggers; }
};
