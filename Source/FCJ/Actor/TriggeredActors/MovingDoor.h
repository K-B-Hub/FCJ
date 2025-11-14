// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "MovingDoor.generated.h"

/**
 * 이동하는 문 액터
 * 외부에서 OpenDoor()를 호출하여 문을 열 수 있습니다.
 * 문은 공중으로 올라가는 방식으로 열리며, 한번 열리면 다시 닫히지 않습니다.
 */
UCLASS()
class FCJ_API AMovingDoor : public AActor
{
	GENERATED_BODY()

public:
	AMovingDoor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 문 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UStaticMeshComponent* DoorMesh;

	// 문이 열려있는지 여부 (한번 true가 되면 영구적으로 true)
	UPROPERTY(ReplicatedUsing = OnRep_IsOpen, VisibleAnywhere, BlueprintReadOnly, Category = "Door State")
	bool bIsOpen;

	// 문이 이동 중인지 여부
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Door State")
	bool bIsMoving;

	// 문 열림 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ToolTip = "문이 이동할 높이 (로컬 Z축 오프셋)"))
	float OpenHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door Settings", meta = (ClampMin = "0.1", ToolTip = "문이 열리는 속도 (단위/초)"))
	float OpenSpeed = 200.0f;

	// 초기 위치 저장
	FVector InitialLocation;
	FVector TargetLocation;

	// bIsOpen 리플리케이션 콜백
	UFUNCTION()
	void OnRep_IsOpen();

public:
	virtual void Tick(float DeltaTime) override;

	/**
	 * 문을 엽니다. 한번 열리면 다시 닫히지 않습니다.
	 * 서버에서만 호출되어야 합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();

	/**
	 * 문이 열려있는지 확인합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	bool IsOpen() const { return bIsOpen; }

	/**
	 * 문이 이동 중인지 확인합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	bool IsMoving() const { return bIsMoving; }
};
