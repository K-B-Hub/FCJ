// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/ClearDoor.h"
#include "Actor/Volumes/ZoneVolume.h"
#include "Actor/TriggeredActors/MovingDoor.h"

AClearDoor::AClearDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// Zone 컴포넌트 생성
	ZoneComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("ZoneComponent"));
	ZoneComponent->SetupAttachment(RootComponent);

	// Door 컴포넌트 생성
	DoorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorComponent"));
	DoorComponent->SetupAttachment(RootComponent);

	CachedZone = nullptr;
	CachedDoor = nullptr;
}

void AClearDoor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s BeginPlay - HasAuthority: %d"), *GetName(), HasAuthority());

	// 자식 액터 캐싱
	if (ZoneComponent)
	{
		CachedZone = Cast<AZoneVolume>(ZoneComponent->GetChildActor());
		if (CachedZone)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s - CachedZone: %s"), *GetName(), *CachedZone->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ClearDoor] %s - Failed to get child Zone actor!"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ClearDoor] %s - ZoneComponent is null!"), *GetName());
	}

	if (DoorComponent)
	{
		CachedDoor = Cast<AMovingDoor>(DoorComponent->GetChildActor());
		if (CachedDoor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s - CachedDoor: %s"), *GetName(), *CachedDoor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ClearDoor] %s - Failed to get child Door actor!"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ClearDoor] %s - DoorComponent is null!"), *GetName());
	}

	// Zone 클리어 델리게이트 바인딩
	if (CachedZone)
	{
		CachedZone->OnZoneCleared.AddDynamic(this, &AClearDoor::OnZoneClearedCallback);
		CachedZone->SetZoneNumber(ZoneNumber);
		UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s - Delegate bound to Zone"), *GetName());
	}
}

void AClearDoor::OnZoneClearedCallback()
{
	UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s OnZoneClearedCallback - HasAuthority: %d, CachedDoor valid: %d"),
		*GetName(), HasAuthority(), (CachedDoor != nullptr));

	// Zone이 클리어되면 문을 엽니다
	if (CachedDoor && HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s - Opening door!"), *GetName());
		CachedDoor->OpenDoor();

		// 한번 문이 열리면 더 이상 델리게이트를 들을 필요가 없음
		if (CachedZone)
		{
			CachedZone->OnZoneCleared.RemoveDynamic(this, &AClearDoor::OnZoneClearedCallback);
			UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s - Delegate unbound"), *GetName());
		}
	}
	else if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ClearDoor] %s - Not authority, skipping door open"), *GetName());
	}
	else if (!CachedDoor)
	{
		UE_LOG(LogTemp, Error, TEXT("[ClearDoor] %s - CachedDoor is null!"), *GetName());
	}
}
