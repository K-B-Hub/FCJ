// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/MovingDoor.h"
#include "Net/UnrealNetwork.h"

AMovingDoor::AMovingDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 리플리케이션 활성화
	bReplicates = true;

	// 문 메시 생성
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	DoorMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	RootComponent = DoorMesh;

	bIsOpen = false;
	bIsMoving = false;
}

void AMovingDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMovingDoor, bIsOpen);
	DOREPLIFETIME(AMovingDoor, bIsMoving);
}

void AMovingDoor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s BeginPlay - HasAuthority: %d"), *GetName(), HasAuthority());

	// 초기 위치 저장
	InitialLocation = DoorMesh->GetRelativeLocation();
	TargetLocation = InitialLocation + FVector(0.0f, 0.0f, OpenHeight);
}

void AMovingDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// bIsOpen 상태에 따라 목표 위치 업데이트
	if (bIsOpen)
	{
		TargetLocation = InitialLocation + FVector(0.0f, 0.0f, OpenHeight);
	}
	else
	{
		TargetLocation = InitialLocation;
	}

	// 현재 위치가 목표 위치와 다르면 이동
	FVector CurrentLocation = DoorMesh->GetRelativeLocation();
	if (!CurrentLocation.Equals(TargetLocation, 1.0f))
	{
		bIsMoving = true;
		FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, OpenSpeed);
		DoorMesh->SetRelativeLocation(NewLocation);

		// 목표 위치에 도달했는지 확인
		if (FVector::Dist(NewLocation, TargetLocation) < 1.0f)
		{
			DoorMesh->SetRelativeLocation(TargetLocation);
			bIsMoving = false;
		}
	}
	else
	{
		bIsMoving = false;
	}
}

void AMovingDoor::OpenDoor()
{
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s OpenDoor called - HasAuthority: %d, bIsOpen: %d"),
		*GetName(), HasAuthority(), bIsOpen);

	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Not authority, returning"), *GetName());
		return;
	}

	// 이미 열려있으면 무시
	if (bIsOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Already open, returning"), *GetName());
		return;
	}

	// 문 열기 시작
	bIsOpen = true;
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Door is now opening!"), *GetName());
}

void AMovingDoor::CloseDoor()
{
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s CloseDoor called - HasAuthority: %d, bIsOpen: %d"),
		*GetName(), HasAuthority(), bIsOpen);

	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Not authority, returning"), *GetName());
		return;
	}

	// 이미 닫혀있으면 무시
	if (!bIsOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Already closed, returning"), *GetName());
		return;
	}

	// 문 닫기 시작
	bIsOpen = false;
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Door is now closing!"), *GetName());
}

void AMovingDoor::OnRep_IsOpen()
{
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s OnRep_IsOpen - bIsOpen: %d"),
		*GetName(), bIsOpen);

	// bIsOpen 상태가 변경되면 Tick에서 자동으로 목표 위치로 이동 시작
	// 별도의 처리 없이 Tick에서 처리됨
}

