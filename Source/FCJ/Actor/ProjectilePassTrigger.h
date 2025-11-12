// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BaseTrigger.h"
#include "Components/BoxComponent.h"
#include "ProjectilePassTrigger.generated.h"

class AProjectile;

/**
 * 발사체 통과 트리거
 * Projectile이 트리거 영역을 한 번이라도 지나가면 영구적으로 true를 반환합니다.
 * bInvertTrigger를 true로 설정하면 발사체가 지나간 후 false를 반환합니다.
 */
UCLASS()
class FCJ_API AProjectilePassTrigger : public ABaseTrigger
{
	GENERATED_BODY()

public:
	AProjectilePassTrigger();

protected:
	virtual void BeginPlay() override;

	// 트리거 영역
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UBoxComponent* TriggerBox;

	// 발사체가 지나갔는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger State")
	bool bProjectilePassed = false;

	// 트리거를 리셋할 수 있는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Settings", meta = (ToolTip = "true면 트리거를 리셋할 수 있습니다"))
	bool bCanReset = false;

public:
	// 내부 트리거 상태 반환 (발사체가 지나갔으면 true)
	virtual bool GetInternalTriggerState_Implementation() const override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 트리거 상태를 리셋합니다
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void ResetTrigger();
};
