// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter/CatBase.h"
#include "AttackCat.generated.h"

class AProjectile;

UCLASS()
class FCJ_API AAttackCat : public ACatBase
{
	GENERATED_BODY()

public:
	AAttackCat();

protected:
	virtual void BeginPlay() override;

	// Attack Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings", meta = (ToolTip = "공격 애니메이션 몽타주"))
	class UAnimMontage* AttackMontage;

private:
	// Parrying state
	bool bIsParrying = false;

	// Timer handle for projectile reflection check
	FTimerHandle ProjectileReflectionTimer;

public:
	// Override special action for attack
	virtual void PerformSpecialAction() override;

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
};
