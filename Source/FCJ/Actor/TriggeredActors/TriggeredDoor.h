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
 * 두 트리거가 모두 활성화되면 문이 열리며, 한번 열리면 다시 닫히지 않습니다.
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USceneComponent* RootSceneComponent;

	// 문 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "문 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* DoorComponent;

	// 첫 번째 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "첫 번째 트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* Trigger1Component;

	// 두 번째 트리거 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "두 번째 트리거 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* Trigger2Component;

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
	 * 두 트리거 상태를 확인하고 필요시 문을 엽니다.
	 */
	void CheckAndOpenDoor();

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
