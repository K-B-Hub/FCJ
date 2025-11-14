// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TimerHandle.h"
#include "Actor/Objects/Projectile.h"
#include "Net/UnrealNetwork.h"
#include "Turret.generated.h"

class ACatBase;

/**
 * 포탑 클래스
 * 범위 내의 플레이어를 감지하고, 랜덤한 타겟을 선택하여 발사체를 발사합니다.
 */
UCLASS()
class FCJ_API ATurret : public AActor
{
	GENERATED_BODY()

public:
	ATurret();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 컴포넌트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USphereComponent* DetectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UStaticMeshComponent* TurretHead;

	// 발사체 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Settings", meta = (ToolTip = "발사할 발사체의 클래스"))
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Settings", meta = (ClampMin = "100.0", ToolTip = "플레이어 감지 반경"))
	float DetectionRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Settings", meta = (ClampMin = "0.1", ToolTip = "발사 간격 (초)"))
	float FireInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Settings", meta = (ClampMin = "0.0", ToolTip = "포탑 헤드 회전 속도 (도/초)"))
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Settings", meta = (ClampMin = "0.0", ToolTip = "발사체가 생성되는 위치 오프셋 (로컬)"))
	FVector ProjectileSpawnOffset = FVector(100.0f, 0.0f, 0.0f);

	// 발사체 타입 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Type Chances", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "완전 호밍 발사체의 생성 확률"))
	float HomingProjectileChance = 0.33f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Type Chances", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "약간 유도되는 발사체의 생성 확률"))
	float SlightGuidedProjectileChance = 0.33f;

protected:
	// 감지된 플레이어들
	UPROPERTY(Replicated)
	TArray<ACatBase*> DetectedPlayers;

	// 현재 타겟
	UPROPERTY(Replicated)
	ACatBase* CurrentTarget;

	// 포탑 활성화 상태
	UPROPERTY(Replicated)
	bool bIsActive;

	FTimerHandle FireTimerHandle;

public:
	virtual void Tick(float DeltaTime) override;

	// 겹침 이벤트 핸들러
	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	// 포탑 활성화/비활성화 (서버에서만 호출, 리플리케이션을 통해 클라이언트 동기화)
	UFUNCTION(BlueprintCallable, Category = "Turret")
	void SetActive(bool bNewActive);

	UFUNCTION(BlueprintCallable, Category = "Turret")
	bool IsActive() const { return bIsActive; }

	// 발사 시각/사운드 효과용 Multicast
	UFUNCTION(NetMulticast, Reliable, Category = "Turret")
	void MulticastOnProjectileFired(AProjectile* NewProjectile);

private:
	void StartFiring();
	void StopFiring();
	void FireProjectile();
	void UpdateTargetRotation(float DeltaTime);
	void SelectRandomTarget();
	EProjectileType GetRandomProjectileType() const;
	void CleanupDetectedPlayers();
};
