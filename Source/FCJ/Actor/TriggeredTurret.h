// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ChildActorComponent.h"
#include "TriggeredTurret.generated.h"

class ATurret;
class ABaseTrigger;

/**
 * 트리거에 의해 제어되는 포탑 시스템
 * Turret과 BaseTrigger를 자식 액터 컴포넌트로 가지며,
 * Blueprint에서 각 컴포넌트의 위치를 시각적으로 조절할 수 있습니다.
 * BaseTrigger의 상태에 따라 Turret을 활성화/비활성화합니다.
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USceneComponent* RootSceneComponent;

	// 포탑 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "포탑 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* TurretComponent;

	// 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* TriggerComponent;

	// 트리거 상태를 자동으로 체크할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings", meta = (ToolTip = "트리거 상태를 자동으로 체크하여 포탑을 제어할지 여부"))
	bool bAutoCheckTrigger = true;

	// 트리거 체크 빈도 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings", meta = (ClampMin = "0.1", ToolTip = "트리거 상태를 체크하는 주기 (초)"))
	float TriggerCheckInterval = 0.1f;

private:
	bool bLastTriggerState;
	float TimeSinceLastCheck;

	// 캐시된 자식 액터들
	UPROPERTY()
	ATurret* CachedTurret;

	UPROPERTY()
	ABaseTrigger* CachedTrigger;

public:
	virtual void Tick(float DeltaTime) override;

	/**
	 * 트리거 상태를 수동으로 체크하고 포탑 상태를 업데이트합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void CheckTriggerState();

	/**
	 * Turret 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	ATurret* GetTurret() const { return CachedTurret; }

	/**
	 * Trigger 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	ABaseTrigger* GetTrigger() const { return CachedTrigger; }
};
