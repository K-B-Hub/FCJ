// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "HoldingObject.generated.h"

class ABiteCat;

UCLASS(Blueprintable)
class FCJ_API AHoldingObject : public AActor
{
	GENERATED_BODY()

public:
	AHoldingObject();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 메시 컴포넌트 (블루프린트에서 다양한 오브젝트 설정 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// 충돌 컴포넌트 (트리거 전용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	// 잡혔는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Holding")
	bool bIsBeingHeld;

	// 현재 잡고 있는 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Holding")
	ABiteCat* HoldingCat;

	// 잡기 가능한 무게 (BiteCat의 최대 무게와 비교)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holding Settings", meta = (ToolTip = "이 오브젝트의 무게입니다. BiteCat의 최대 무게보다 작아야 잡을 수 있습니다"))
	float Weight = 50.0f;

	// 잡혔을 때의 오프셋 (BiteCat 기준 상대 위치)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holding Settings", meta = (ToolTip = "잡혔을 때 BiteCat을 기준으로 한 상대 위치를 설정합니다"))
	FVector HoldOffset = FVector(150.0f, 0.0f, 0.0f);

	// 콜리전 구체 반경 설정 (트리거 전용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings", meta = (ToolTip = "오브젝트의 트리거 콜리전 구체 반경을 설정합니다"))
	float SphereRadius = 50.0f;

	// 물리 댐핑 설정 (낮게 설정하여 중력이 정상적으로 작동하도록)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ToolTip = "오브젝트의 선형 댐핑값입니다. 높을수록 움직임이 안정적입니다"))
	float LinearDamping = 1.0f; // 낮은 선형 댐핑으로 중력 정상화

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ToolTip = "오브젝트의 각속도 댐핑값입니다. 높을수록 회전이 안정적입니다"))
	float AngularDamping = 1.0f; // 낮은 각속도 댐핑

	// 물리 안정성 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ToolTip = "물체의 안정성을 높입니다. 높을수록 외부 충격에 덜 반응합니다"))
	float StabilityDamping = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ToolTip = "중력 스케일 값입니다. 1.0이 기본 중력입니다"))
	float GravityScale = 1.0f;

	// 리스폰 관련 설정
	UPROPERTY()
	FVector InitialLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Settings", meta = (ToolTip = "초기 위치에서 이 거리 이상 아래로 떨어지면 리스폰됩니다 (양수 값)"))
	float RespawnZThreshold = 500.0f;

public:
	virtual void Tick(float DeltaTime) override;

	// 잡기 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool CanBeHeld() const;

	UFUNCTION(BlueprintCallable, Category = "Holding")
	void OnHeld(ABiteCat* Cat);

	UFUNCTION(BlueprintCallable, Category = "Holding")
	void OnReleased();


	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool IsBeingHeld() const { return bIsBeingHeld; }

	UFUNCTION(BlueprintCallable, Category = "Holding")
	float GetWeight() const { return Weight; }

	UFUNCTION(BlueprintCallable, Category = "Holding")
	FVector GetHoldOffset() const { return HoldOffset; }

	UFUNCTION(BlueprintCallable, Category = "Holding")
	ABiteCat* GetHoldingCat() const { return HoldingCat; }

	// 리스폰 함수 (RespawnVolume에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnToInitialLocation();

};
