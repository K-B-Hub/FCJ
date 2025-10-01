// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter/CatBase.h"
#include "Net/UnrealNetwork.h"
#include "BiteCat.generated.h"

class AHoldingObject;

UCLASS(Blueprintable)
class FCJ_API ABiteCat : public ACatBase
{
	GENERATED_BODY()
	
public:
	ABiteCat();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 현재 잡고 있는 오브젝트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Holding")
	AHoldingObject* CurrentHeldObject;
	
	// 물체를 던질 때의 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holding Settings", meta = (ToolTip = "물체를 던질 때 가해지는 힘의 크기를 설정합니다"))
	float ThrowForce = 1200.0f;

public:
	// PerformSpecialAction 오버라이드 (잡기 기능)
	virtual void PerformSpecialAction() override;

	// 잡기 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Holding")
	AHoldingObject* FindNearestHoldableObject() const;

	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool CanHoldObject(AHoldingObject* Object) const;

	UFUNCTION(BlueprintCallable, Category = "Holding")
	void HoldObject(AHoldingObject* Object);


	UFUNCTION(BlueprintCallable, Category = "Holding")
	void ThrowObject();

	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool IsHoldingObject() const { return CurrentHeldObject != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Holding")
	AHoldingObject* GetHeldObject() const { return CurrentHeldObject; }

	// 서버 RPC 함수들
	UFUNCTION(Server, Reliable, Category = "Holding")
	void ServerHoldObject(AHoldingObject* Object);

	UFUNCTION(Server, Reliable, Category = "Holding")
	void ServerThrowObject();
};
