// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/OneTriggeredDoor.h"
#include "Actor/TriggeredActors/MovingDoor.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Components/ChildActorComponent.h"

AOneTriggeredDoor::AOneTriggeredDoor()
{
	// 델리게이트 기반 이벤트 처리를 사용하므로 Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Door 자식 액터 컴포넌트 생성
	DoorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorComponent"));
	DoorComponent->SetupAttachment(RootSceneComponent);
	// Door 클래스는 Blueprint에서 설정 (Child Actor Class)

	// Trigger 자식 액터 컴포넌트 생성
	TriggerComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TriggerComponent"));
	TriggerComponent->SetupAttachment(RootSceneComponent);
	// Trigger 클래스는 Blueprint에서 설정 (Child Actor Class)

	CachedDoor = nullptr;
	CachedTrigger = nullptr;
}

void AOneTriggeredDoor::BeginPlay()
{
	Super::BeginPlay();

	// 자식 액터들을 캐시
	if (DoorComponent)
	{
		CachedDoor = Cast<AMovingDoor>(DoorComponent->GetChildActor());

		// 문 설정 적용
		if (CachedDoor)
		{
			CachedDoor->OpenHeight = OpenHeight;
		 	CachedDoor->OpenSpeed = OpenSpeed;
		}
	}

	if (TriggerComponent)
	{
		CachedTrigger = Cast<ABaseTrigger>(TriggerComponent->GetChildActor());
	}

	// 트리거 델리게이트 바인딩
	if (CachedTrigger)
	{
		CachedTrigger->OnTriggerStateChanged.AddDynamic(this, &AOneTriggeredDoor::OnTriggerStateChanged);
	}
}

void AOneTriggeredDoor::OnTriggerStateChanged(bool bNewState)
{
	if (!CachedDoor)
	{
		return;
	}

	// 트리거 상태에 따라 문 열기/닫기
	if (bNewState)
	{
		// 트리거 활성화 -> 문 열기
		CachedDoor->OpenDoor();
	}
	else
	{
		// 트리거 비활성화 -> 문 닫기
		CachedDoor->CloseDoor();
	}
}
