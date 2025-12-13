// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ChildActorComponent.h"
#include "TriggeredDoor.generated.h"

class AMovingDoor;
class ABaseTrigger;

/**
 * 트리거에 의해 제어되는 문 시스템
 * MovingDoor와 2개의 BaseTrigger를 자식 액터 컴포넌트로 가지며,
 * Blueprint에서 각 컴포넌트의 위치를 시각적으로 조절할 수 있습니다.
 * AND 연산: 두 트리거가 모두 활성화되면 문이 열림
 * OR 연산: 두 트리거 중 하나라도 활성화되면 문이 열림
 * bCanReopen=true: 트리거 상태에 따라 문이 열렸다 닫힘
 * bCanReopen=false: 한번 열리면 영구적으로 열려있음
 */
UCLASS()
class FCJ_API ATriggeredDoor : public AActor
{
	GENERATED_BODY()

public:
	ATriggeredDoor();

protected:
	virtual void BeginPlay() override;

	// 루트 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components)
	USceneComponent* RootSceneComponent;

	// 문 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "문 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* DoorComponent;

	// 첫 번째 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "첫 번째 트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* Trigger1Component;

	// 두 번째 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Components, meta = (ToolTip = "두 번째 트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* Trigger2Component;

	// 트리거 로직 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ToolTip = "false: AND 연산 (둘 다 활성화), true: OR 연산 (하나라도 활성화)"))
	bool bUseORLogic = false;

	// 문 재개방 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ToolTip = "false: 한번 열리면 영구적으로 열림, true: 트리거 상태에 따라 열고 닫힘"))
	bool bCanReopen = false;

	// 문 이동 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ToolTip = "문이 이동할 높이 (로컬 Z축 오프셋)"))
	float OpenHeight = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ClampMin = "0.1", ToolTip = "문이 열리는 속도 (단위/초)"))
	float OpenSpeed = 200.0f;

private:
	// 캐시된 자식 액터들
	UPROPERTY()
	AMovingDoor* CachedDoor;

	UPROPERTY()
	ABaseTrigger* CachedTrigger1;

	UPROPERTY()
	ABaseTrigger* CachedTrigger2;

	// 각 트리거의 현재 상태
	bool bTrigger1Active;
	bool bTrigger2Active;

	// 문이 이미 열렸는지 여부 (한번 열리면 영구적으로 true)
	bool bDoorOpened;

	/**
	 * 첫 번째 트리거 상태 변경 시 호출되는 콜백 함수
	 * @param bNewState 새로운 트리거 상태
	 */
	UFUNCTION()
	void OnTrigger1StateChanged(bool bNewState);

	/**
	 * 두 번째 트리거 상태 변경 시 호출되는 콜백 함수
	 * @param bNewState 새로운 트리거 상태
	 */
	UFUNCTION()
	void OnTrigger2StateChanged(bool bNewState);

	/**
	 * 두 트리거 상태를 확인하고 필요시 문을 열거나 닫습니다.
	 */
	void CheckAndUpdateDoor();

public:
	/**
	 * Door 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	AMovingDoor* GetDoor() const { return CachedDoor; }

	/**
	 * 첫 번째 Trigger 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	ABaseTrigger* GetTrigger1() const { return CachedTrigger1; }

	/**
	 * 두 번째 Trigger 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	ABaseTrigger* GetTrigger2() const { return CachedTrigger2; }

	/**
	 * 문이 열렸는지 확인합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	bool IsDoorOpened() const { return bDoorOpened; }
};
