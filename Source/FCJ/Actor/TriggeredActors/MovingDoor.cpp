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

	// 문이 이동 중일 때만 Tick에서 처리
	if (bIsMoving)
	{
		FVector CurrentLocation = DoorMesh->GetRelativeLocation();
		FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, OpenSpeed);
		DoorMesh->SetRelativeLocation(NewLocation);

		// 목표 위치에 도달했는지 확인
		if (FVector::Dist(NewLocation, TargetLocation) < 1.0f)
		{
			DoorMesh->SetRelativeLocation(TargetLocation);
			bIsMoving = false;
		}
	}
}

void AMovingDoor::OpenDoor()
{
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s OpenDoor called - HasAuthority: %d, bIsOpen: %d, bIsMoving: %d"),
		*GetName(), HasAuthority(), bIsOpen, bIsMoving);

	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Not authority, returning"), *GetName());
		return;
	}

	// 이미 열려있거나 이동 중이면 무시
	if (bIsOpen || bIsMoving)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Already open or moving, returning"), *GetName());
		return;
	}

	// 문 열기 시작
	bIsOpen = true;
	bIsMoving = true;
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Door is now opening!"), *GetName());
}

void AMovingDoor::OnRep_IsOpen()
{
	UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s OnRep_IsOpen - bIsOpen: %d, bIsMoving: %d"),
		*GetName(), bIsOpen, bIsMoving);

	// 클라이언트에서 리플리케이션을 통해 문이 열렸을 때 처리
	if (bIsOpen && !bIsMoving)
	{
		// 서버에서 이미 이동이 완료된 경우, 클라이언트도 즉시 목표 위치로 이동
		DoorMesh->SetRelativeLocation(TargetLocation);
		UE_LOG(LogTemp, Warning, TEXT("[MovingDoor] %s - Door position updated on client"), *GetName());
	}
}

