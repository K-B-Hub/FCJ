// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
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

	// 메시 컴포넌트 (블루프린트에서 다양한 오브젝트 설정 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionComponent;

	// 잡혔는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Holding")
	bool bIsBeingHeld;

	// 현재 잡고 있는 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Holding")
	ABiteCat* HoldingCat;

	// 잡기 가능한 무게 (BiteCat의 최대 무게와 비교)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holding Settings", meta = (ToolTip = "이 오브젝트의 무게입니다. BiteCat의 최대 무게보다 작아야 잡을 수 있습니다"))
	float Weight = 1.0f;

	// 잡혔을 때의 오프셋 (BiteCat 기준 상대 위치)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holding Settings", meta = (ToolTip = "잡혔을 때 BiteCat을 기준으로 한 상대 위치를 설정합니다"))
	FVector HoldOffset = FVector(100.0f, 0.0f, 0.0f);

	// 콜리전 박스 크기 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Settings", meta = (ToolTip = "오브젝트의 콜리전 박스 크기를 설정합니다"))
	FVector BoxExtent = FVector(50.0f, 50.0f, 50.0f);

	// 물리 댐핑 설정 (높을수록 더 안정적)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ToolTip = "오브젝트의 선형 댐핑값입니다. 높을수록 움직임이 안정적입니다"))
	float LinearDamping = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ToolTip = "오브젝트의 각속도 댐핑값입니다. 높을수록 회전이 안정적입니다"))
	float AngularDamping = 1.0f;


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
};
