// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter/CatBase.h"
#include "BiteCat.generated.h"

class AHoldingObject;

UCLASS(Blueprintable)
class FCJ_API ABiteCat : public ACatBase
{
	GENERATED_BODY()
	
public:
	ABiteCat();

protected:
	// 현재 잡고 있는 오브젝트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Holding")
	AHoldingObject* CurrentHeldObject;

	// 잡을 수 있는 최대 무게
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holding Settings", meta = (ToolTip = "BiteCat이 잡을 수 있는 최대 무게를 설정합니다"))
	float MaxHoldWeight = 2.0f;

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
	void ReleaseObject();

	UFUNCTION(BlueprintCallable, Category = "Holding")
	bool IsHoldingObject() const { return CurrentHeldObject != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Holding")
	AHoldingObject* GetHeldObject() const { return CurrentHeldObject; }
};
