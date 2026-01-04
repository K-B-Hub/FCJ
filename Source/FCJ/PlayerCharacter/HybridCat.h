// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter/CatBase.h"
#include "Net/UnrealNetwork.h"
#include "HybridCat.generated.h"

class AHoldingObject;
class AProjectile;

/**
 * HybridCat - Combines BiteCat (Grab/Throw) and AttackCat (Parry/Push) abilities
 *
 * Input System:
 * - SpecialAction (LeftShift): Grab/Throw functionality
 * - SecondarySpecialAction (LeftControl): Parry/Push functionality
 *
 * Mutual Exclusion:
 * - Cannot parry while holding an object
 * - Cannot grab while parrying
 */
UCLASS(Blueprintable)
class FCJ_API AHybridCat : public ACatBase
{
	GENERATED_BODY()

public:
	AHybridCat();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========== BiteCat Functionality ==========

	// 현재 잡고 있는 오브젝트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Holding")
	AHoldingObject* CurrentHeldObject;

	// 충전 시스템 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Settings", meta = (ToolTip = "최소 던지기 힘 (충전하지 않았을 때)"))
	float MinThrowForce = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Settings", meta = (ToolTip = "최대 던지기 힘 (최대 충전 시)"))
	float MaxThrowForce = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge Settings", meta = (ToolTip = "최대 충전 시간 (초)"))
	float MaxChargeTime = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Charge Settings", meta = (ToolTip = "현재 충전 중인지 여부"))
	bool bIsCharging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Charge Settings", meta = (ToolTip = "현재 충전 시간"))
	float CurrentChargeTime = 0.0f;

	// ========== AttackCat Functionality ==========

	// Attack Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings", meta = (ToolTip = "공격/패링 애니메이션 몽타주"))
	class UAnimMontage* AttackMontage;

	// Push Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings", meta = (ToolTip = "패링 시 물체를 밀어내는 힘"))
	float PushForce = 1500.0f;

private:
	// Parrying state
	bool bIsParrying = false;

	// Timer handle for projectile reflection check
	FTimerHandle ProjectileReflectionTimer;

public:
	// ========== Core Overrides ==========

	virtual void Tick(float DeltaTime) override;

	// Override special actions with mutual exclusion logic
	virtual void PerformSpecialAction() override;
	virtual void PerformSecondarySpecialAction() override;
	virtual void OnSpecialActionReleased() override;

	// ========== BiteCat Functions ==========

	// 잡기 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Holding")
	AHoldingObject* FindNearestHoldableObject() const;

	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool CanHoldObject(AHoldingObject* Object) const;

	UFUNCTION(BlueprintCallable, Category = "Holding")
	void HoldObject(AHoldingObject* Object);

	// 충전 시스템 함수들
	UFUNCTION(BlueprintCallable, Category = "Charge")
	void StartCharging();

	UFUNCTION(BlueprintCallable, Category = "Charge")
	void ReleaseThrow();

	UFUNCTION(BlueprintCallable, Category = "Charge")
	float CalculateThrowForce() const;

	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool IsHoldingObject() const { return CurrentHeldObject != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Holding")
	AHoldingObject* GetHeldObject() const { return CurrentHeldObject; }

	// 서버 RPC 함수들 (BiteCat)
	UFUNCTION(Server, Reliable, Category = "Holding")
	void ServerHoldObject(AHoldingObject* Object);

	UFUNCTION(Server, Reliable, Category = "Charge")
	void ServerStartCharging();

	UFUNCTION(Server, Reliable, Category = "Charge")
	void ServerReleaseThrow(float ChargeTime);

	// ========== AttackCat Functions ==========

	// Parrying functions
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StartParrying();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopParrying();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	bool IsParrying() const { return bIsParrying; }

	// Projectile reflection logic
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void CheckAndReflectProjectiles();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ReflectProjectile(AProjectile* Projectile);

	// Object pushing logic
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void PushNearbyObjects();

	UFUNCTION(Server, Reliable, Category = "Attack")
	void ServerPushNearbyObjects();
};
