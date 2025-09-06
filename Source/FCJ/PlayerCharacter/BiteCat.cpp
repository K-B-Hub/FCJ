// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/BiteCat.h"
#include "Actor/HoldingObject.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ABiteCat::ABiteCat()
{
	CurrentHeldObject = nullptr;
}

void ABiteCat::PerformSpecialAction()
{
	UE_LOG(LogTemp, Warning, TEXT("BiteCat PerformSpecialAction called"));

	// BiteCat의 특수 행동: 잡기/놓기
	if (IsHoldingObject())
	{
		// 현재 오브젝트를 들고 있다면 놓기
		UE_LOG(LogTemp, Warning, TEXT("Releasing held object"));
		ReleaseObject();
	}
	else
	{
		// 오브젝트를 들고 있지 않다면 가장 가까운 오브젝트 잡기 시도
		AHoldingObject* NearestObject = FindNearestHoldableObject();
		UE_LOG(LogTemp, Warning, TEXT("Found nearest object: %s"), NearestObject ? *NearestObject->GetName() : TEXT("None"));
		
		if (NearestObject && CanHoldObject(NearestObject))
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempting to hold object"));
			HoldObject(NearestObject);
		}
		else if (NearestObject)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot hold object - Weight: %f, MaxWeight: %f"), 
				NearestObject->GetWeight(), MaxHoldWeight);
		}
	}

	// 부모 클래스의 Blueprint 이벤트도 호출
	Super::PerformSpecialAction();
}

AHoldingObject* ABiteCat::FindNearestHoldableObject() const
{
	if (!SpecialActionBox)
	{
		return nullptr;
	}

	// SpecialActionBox와 오버랩된 HoldingObject들 가져오기
	TArray<AActor*> OverlappingActors;
	SpecialActionBox->GetOverlappingActors(OverlappingActors, AHoldingObject::StaticClass());

	AHoldingObject* NearestObject = nullptr;
	float MinDistance = FLT_MAX;

	for (AActor* Actor : OverlappingActors)
	{
		if (AHoldingObject* HoldingObject = Cast<AHoldingObject>(Actor))
		{
			if (HoldingObject->CanBeHeld())
			{
				float Distance = FVector::Dist(GetActorLocation(), HoldingObject->GetActorLocation());
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					NearestObject = HoldingObject;
				}
			}
		}
	}

	return NearestObject;
}

bool ABiteCat::CanHoldObject(AHoldingObject* Object) const
{
	if (!Object)
	{
		return false;
	}

	// 이미 다른 오브젝트를 들고 있다면 불가능
	if (IsHoldingObject())
	{
		return false;
	}

	// 오브젝트가 이미 잡혀있다면 불가능
	if (!Object->CanBeHeld())
	{
		return false;
	}

	// 무게 제한 체크
	if (Object->GetWeight() > MaxHoldWeight)
	{
		return false;
	}

	return true;
}

void ABiteCat::HoldObject(AHoldingObject* Object)
{
	if (!CanHoldObject(Object))
	{
		return;
	}

	CurrentHeldObject = Object;
	Object->OnHeld(this);

	UE_LOG(LogTemp, Warning, TEXT("BiteCat grabbed object: %s"), *Object->GetName());
}

void ABiteCat::ReleaseObject()
{
	if (!CurrentHeldObject)
	{
		return;
	}

	CurrentHeldObject->OnReleased();
	CurrentHeldObject = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("BiteCat released object"));
}

