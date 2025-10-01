// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/BiteCat.h"
#include "Actor/HoldingObject.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ABiteCat::ABiteCat()
{
	CurrentHeldObject = nullptr;
}

void ABiteCat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABiteCat, CurrentHeldObject);
}

void ABiteCat::PerformSpecialAction()
{
	UE_LOG(LogTemp, Warning, TEXT("BiteCat PerformSpecialAction called"));

	// BiteCat의 특수 행동: 잡기/던지기
	if (IsHoldingObject())
	{
		// 현재 오브젝트를 들고 있다면 던지기
		UE_LOG(LogTemp, Warning, TEXT("Throwing held object"));
		ThrowObject();
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
			UE_LOG(LogTemp, Warning, TEXT("Cannot hold object - Object may already be held or other restriction"));
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

	// 무게 제한 체크 제거 - 모든 HoldingObject를 잡을 수 있음
	// if (Object->GetWeight() > MaxHoldWeight)
	// {
	// 	return false;
	// }

	return true;
}

void ABiteCat::HoldObject(AHoldingObject* Object)
{
	if (!CanHoldObject(Object))
	{
		return;
	}

	// 서버에 요청 전송
	ServerHoldObject(Object);
}

void ABiteCat::ServerHoldObject_Implementation(AHoldingObject* Object)
{
	if (!CanHoldObject(Object))
	{
		return;
	}

	CurrentHeldObject = Object;
	Object->OnHeld(this);

	UE_LOG(LogTemp, Warning, TEXT("BiteCat grabbed object: %s"), *Object->GetName());
}


void ABiteCat::ThrowObject()
{
	if (!CurrentHeldObject)
	{
		return;
	}

	// 서버에 요청 전송
	ServerThrowObject();
}

void ABiteCat::ServerThrowObject_Implementation()
{
	if (!CurrentHeldObject)
	{
		return;
	}

	// 캐릭터의 전방 방향 계산
	FVector ForwardDirection = GetActorForwardVector();

	// 던질 방향에 약간의 위쪽 각도 추가 (포물선 궤적을 위해)
	FVector ThrowDirection = ForwardDirection + FVector(0.0f, 0.0f, 0.3f);
	ThrowDirection.Normalize();

	// 오브젝트를 놓기
	AHoldingObject* ObjectToThrow = CurrentHeldObject;
	CurrentHeldObject->OnReleased();
	CurrentHeldObject = nullptr;

	// HoldingObject는 CollisionComponent에 물리가 설정되어 있으므로 CollisionComponent에 힘을 가함
	if (UBoxComponent* CollisionComp = ObjectToThrow->FindComponentByClass<UBoxComponent>())
	{
		if (CollisionComp->IsSimulatingPhysics())
		{
			// 임펄스로 던지기
			FVector ThrowImpulse = ThrowDirection * ThrowForce * CollisionComp->GetMass();
			CollisionComp->AddImpulse(ThrowImpulse);
		}
		else
		{
			// 물리 시뮬레이션이 비활성화되어 있다면 활성화하고 던지기
			CollisionComp->SetSimulatePhysics(true);
			FVector ThrowImpulse = ThrowDirection * ThrowForce * CollisionComp->GetMass();
			CollisionComp->AddImpulse(ThrowImpulse);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BiteCat threw object in direction: %s"), *ThrowDirection.ToString());
}

