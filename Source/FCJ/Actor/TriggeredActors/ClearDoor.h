// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ChildActorComponent.h"
#include "ClearDoor.generated.h"

class AZoneVolume;
class AMovingDoor;

/**
 * Zone 클리어에 의해 제어되는 문 시스템
 * ZoneVolume과 MovingDoor를 자식 액터 컴포넌트로 가지며,
 * Blueprint에서 각 컴포넌트의 위치를 시각적으로 조절할 수 있습니다.
 * Zone이 클리어되면 문이 열립니다.
 */
UCLASS()
class FCJ_API AClearDoor : public AActor
{
	GENERATED_BODY()

public:
	AClearDoor();

protected:
	virtual void BeginPlay() override;

	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USceneComponent* RootSceneComponent;

	// Zone 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "Zone 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* ZoneComponent;

	// 문 자식 액터 컴포넌트 (Blueprint에서 위치/회전 조절 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (ToolTip = "문 액터 컴포넌트 - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* DoorComponent;

private:
	// 캐시된 자식 액터들
	UPROPERTY()
	AZoneVolume* CachedZone;

	UPROPERTY()
	AMovingDoor* CachedDoor;

	/**
	 * Zone 클리어 시 호출되는 콜백 함수
	 */
	UFUNCTION()
	void OnZoneClearedCallback();

public:
	/**
	 * Zone 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	AZoneVolume* GetZone() const { return CachedZone; }

	/**
	 * Door 액터를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	AMovingDoor* GetDoor() const { return CachedDoor; }
};
