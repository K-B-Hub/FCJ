// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Components/BoxComponent.h"
#include "PlayerOverlapTrigger.generated.h"

class ACatBase;

/**
 * 플레이어 오버랩 트리거
 * 플레이어가 트리거 영역에 오버랩되어 있으면 true를 반환합니다.
 * bInvertTrigger를 true로 설정하면 플레이어가 오버랩되어 있을 때 false를 반환합니다.
 */
UCLASS()
class FCJ_API APlayerOverlapTrigger : public ABaseTrigger
{
	GENERATED_BODY()

public:
	APlayerOverlapTrigger();

protected:
	virtual void BeginPlay() override;

	// 트리거 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UBoxComponent* TriggerBox;

	// 오버랩된 플레이어 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger State")
	TArray<ACatBase*> OverlappingPlayers;

public:
	// 내부 트리거 상태 반환 (플레이어가 오버랩되어 있으면 true)
	virtual bool GetInternalTriggerState_Implementation() const override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);
};
