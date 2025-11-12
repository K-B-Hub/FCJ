// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BaseTrigger.h"
#include "Components/BoxComponent.h"
#include "HoldingObjectOverlapTrigger.generated.h"

class AHoldingObject;

/**
 * HoldingObject 오버랩 트리거
 * HoldingObject가 트리거 영역에 오버랩되어 있으면 true를 반환합니다.
 * bInvertTrigger를 true로 설정하면 HoldingObject가 오버랩되어 있을 때 false를 반환합니다.
 */
UCLASS()
class FCJ_API AHoldingObjectOverlapTrigger : public ABaseTrigger
{
	GENERATED_BODY()

public:
	AHoldingObjectOverlapTrigger();

protected:
	virtual void BeginPlay() override;

	// 트리거 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UBoxComponent* TriggerBox;

	// 오버랩된 HoldingObject 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger State")
	TArray<AHoldingObject*> OverlappingObjects;

public:
	// 내부 트리거 상태 반환 (HoldingObject가 오버랩되어 있으면 true)
	virtual bool GetInternalTriggerState_Implementation() const override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);
};
