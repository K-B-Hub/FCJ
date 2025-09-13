// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.generated.h"

class ACatBase;

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Straight UMETA(DisplayName = "Straight"),
	Homing UMETA(DisplayName = "Homing"),
	SlightGuided UMETA(DisplayName = "Slight Guided")
};

UCLASS()
class FCJ_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ToolTip = "발사체의 유형 (직선, 완전 호밍, 약간 유도)"))
	EProjectileType ProjectileType = EProjectileType::Straight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "발사체의 이동 속도"))
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "호밍 발사체의 가속도 크기"))
	float HomingAcceleration = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "약간 유도되는 발사체의 유도 강도"))
	float SlightGuidanceStrength = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.1", ToolTip = "발사체의 수명 (초 단위)"))
	float LifeTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "발사체가 가하는 데미지"))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "캐릭터를 밀어내는 힘의 크기"))
	float KnockbackForce = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "발사체가 볼륨으로부터 날아갈 수 있는 최대 거리"))
	float MaxDistanceFromVolume = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Settings", meta = (ClampMin = "0.0", ToolTip = "상향 넉백 힘의 크기"))
	float VerticalKnockbackForce = 300.0f;

private:
	UPROPERTY()
	ACatBase* TargetCharacter;

	UPROPERTY()
	class AProjectileVolume* SpawnVolume;

	FVector SpawnLocation;

	// Parried projectiles should not home to target anymore
	bool bIsParried = false;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitializeProjectile(EProjectileType InProjectileType, ACatBase* InTarget = nullptr, FVector InDirection = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetSpawnVolume(class AProjectileVolume* Volume);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SetParried(bool InParried = true) { bIsParried = InParried; }

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	bool IsParried() const { return bIsParried; }

private:
	void UpdateMovement(float DeltaTime);
	void HandleHomingMovement(float DeltaTime);
	void HandleSlightGuidedMovement(float DeltaTime);

	FVector InitialDirection;
	float CurrentLifeTime;
};
