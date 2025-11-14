// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Engine/TimerHandle.h"
#include "Actor/Objects/Projectile.h"
#include "Net/UnrealNetwork.h"
#include "ProjectileVolume.generated.h"

class ACatBase;

UENUM(BlueprintType)
enum class EProjectileLaunchMode : uint8
{
	TargetedOnly UMETA(DisplayName = "Targeted Only"),
	RandomOnly UMETA(DisplayName = "Random Only"),
	Mixed UMETA(DisplayName = "Mixed")
};

UCLASS()
class FCJ_API AProjectileVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileVolume();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UBoxComponent* VolumeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ToolTip = "생성할 발사체의 클래스 타입"))
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ToolTip = "발사 모드 (타겟팅만: 플레이어 추적, 랜덤만: 무작위 방향, 혼합: 둘 다 사용)"))
	EProjectileLaunchMode LaunchMode = EProjectileLaunchMode::Mixed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.1", ToolTip = "발사체가 생성되는 간격 (초 단위)"))
	float LaunchInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "1", ToolTip = "동시에 활성화 가능한 최대 발사체 개수"))
	int32 MaxSimultaneousProjectiles = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "발사체가 볼륨 경계로부터 생성되는 거리 (유닛)"))
	float ProjectileSpawnDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "플레이어를 타겟팅하는 발사체의 생성 확률 (0.0 = 0%, 1.0 = 100%)"))
	float TargetedProjectileChance = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Type Chances", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "완전 호밍 발사체의 생성 확률 (플레이어를 정확하게 추적)"))
	float HomingProjectileChance = 0.33f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Type Chances", meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "약간 유도되는 발사체의 생성 확률 (플레이어 방향으로 살짝 유도됨)"))
	float SlightGuidedProjectileChance = 0.33f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ToolTip = "플레이어가 볼륨 영역에 들어왔을 때 자동으로 발사체 생성을 시작할지 여부"))
	bool bActivateOnOverlap = true;

private:
	UPROPERTY(Replicated)
	TArray<ACatBase*> OverlappingCats;

	UPROPERTY(Replicated)
	TArray<AProjectile*> ActiveProjectiles;

	FTimerHandle LaunchTimerHandle;
	UPROPERTY(Replicated)
	bool bIsActive;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Projectile Volume")
	void SetActive(bool bNewActive);

	// 서버 RPC 함수들
	UFUNCTION(Server, Reliable, Category = "Projectile Volume")
	void ServerSetActive(bool bNewActive);

	UFUNCTION(Server, Reliable, Category = "Projectile Volume")
	void ServerLaunchProjectile();

	UFUNCTION(NetMulticast, Reliable, Category = "Projectile Volume")
	void MulticastOnProjectileLaunched(AProjectile* NewProjectile);

	UFUNCTION(BlueprintCallable, Category = "Projectile Volume")
	bool IsActive() const { return bIsActive; }

private:
	void StartLaunching();
	void StopLaunching();
	void LaunchProjectile();
	
	FVector GetRandomLaunchPoint() const;
	FVector GetLaunchPointTowards(const FVector& TargetLocation) const;
	FVector GetValidLaunchDirection(const FVector& LaunchPoint, const FVector& TargetLocation) const;
	EProjectileType GetRandomProjectileType() const;
	ACatBase* GetRandomOverlappingCat() const;
	
	void CleanupActiveProjectiles();
};
