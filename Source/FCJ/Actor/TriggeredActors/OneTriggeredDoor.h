// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ChildActorComponent.h"
#include "Actor/TriggeredActors/MovingDoor.h"
#include "OneTriggeredDoor.generated.h"

class AMovingDoor;
class ABaseTrigger;


/**
 * 단일 트리거에 의해 제어되는 문 시스템
 * MovingDoor와 1개의 BaseTrigger를 자식 액터 컴포넌트로 가지며,
 * Blueprint에서 각 컴포넌트의 위치를 시각적으로 조절할 수 있습니다.
 * 트리거가 활성화되면 문이 열리고, 비활성화되면 문이 닫힙니다.
 */
UCLASS()
class FCJ_API AOneTriggeredDoor : public AActor
{
	GENERATED_BODY()

public:
	AOneTriggeredDoor();

protected:
	virtual void BeginPlay() override;

	// 루트 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
	USceneComponent* RootSceneComponent;

	// 문 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "문 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* DoorComponent;

	// 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* TriggerComponent;

	// 문 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ToolTip = "문이 이동할 높이 (로컬 Z축 오프셋)"))
	float OpenHeight = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ClampMin = "0.1", ToolTip = "문이 열리는 속도 (단위/초)"))
	float OpenSpeed = 200.0f;

private:
	// 캐시된 자식 액터들
	UPROPERTY()
	AMovingDoor* CachedDoor;

	UPROPERTY()
	ABaseTrigger* CachedTrigger;

	/**
	 * 트리거 상태 변경 시 호출되는 콜백 함수
	 * @param bNewState 새로운 트리거 상태
	 */
	UFUNCTION()
	void OnTriggerStateChanged(bool bNewState);

public:
	/**
	 * Door 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	AMovingDoor* GetDoor() const { return CachedDoor; }

	/**
	 * Trigger 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	ABaseTrigger* GetTrigger() const { return CachedTrigger; }

	/**
	 * 문이 열려있는지 확인합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	bool IsDoorOpen() const { return CachedDoor ? CachedDoor->IsOpen() : false; }
};
