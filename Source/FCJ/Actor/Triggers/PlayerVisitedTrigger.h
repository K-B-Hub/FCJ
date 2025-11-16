// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "PlayerVisitedTrigger.generated.h"

class ACatBase;

/**
 * 플레이어 방문 트리거
 * 플레이어가 트리거 영역을 한 번이라도 방문하면 영구적으로 true를 반환합니다.
 * bInvertTrigger를 true로 설정하면 플레이어가 방문한 후 false를 반환합니다.
 */
UCLASS()
class FCJ_API APlayerVisitedTrigger : public ABaseTrigger
{
	GENERATED_BODY()

public:
	APlayerVisitedTrigger();

protected:
	virtual void BeginPlay() override;

	// 플레이어가 방문했는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger State")
	bool bPlayerVisited = false;

	// 트리거를 리셋할 수 있는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings", meta = (ToolTip = "true면 트리거를 리셋할 수 있습니다"))
	bool bCanReset = false;

public:
	// 내부 트리거 상태 반환 (플레이어가 방문했으면 true)
	virtual bool GetInternalTriggerState_Implementation() const override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 트리거 상태를 리셋합니다
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void ResetTrigger();
};
