// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/BaseTrigger.h"
#include "Net/UnrealNetwork.h"

ABaseTrigger::ABaseTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsActive = false;

	// 네트워크 리플리케이션 활성화
	bReplicates = true;
	
	// 시각적 힌트를 위한 스태틱 메시 생성
	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TriggerMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TriggerMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	TriggerMesh->SetRelativeLocation(FVector::ZeroVector);
	TriggerMesh->SetRelativeRotation(FRotator::ZeroRotator);
	TriggerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	TriggerMesh->OverlayMaterialMaxDrawDistance = 5000.0f;
	RootComponent = TriggerMesh;
	
	// 트리거 박스 생성 (기본 설정 - 파생 클래스에서 필요에 따라 오버라이드 가능)
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetupAttachment(RootComponent);
}

void ABaseTrigger::BeginPlay()
{
	Super::BeginPlay();

	// 초기 머테리얼 설정
	UpdateTriggerMaterial();
}

void ABaseTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABaseTrigger::IsTriggerActive_Implementation() const
{
	// 내부 트리거 상태를 가져와서 반전 여부에 따라 반환
	bool InternalState = GetInternalTriggerState();
	return bInvertTrigger ? !InternalState : InternalState;
}

bool ABaseTrigger::GetInternalTriggerState_Implementation() const
{
	// 기본 구현은 bIsActive 값을 반환합니다.
	// 파생 클래스에서 오버라이드하여 복잡한 조건을 구현할 수 있습니다.
	return bIsActive;
}

void ABaseTrigger::SetTriggerActive(bool bNewActive)
{
	// 서버에서만 상태 변경 가능
	if (!HasAuthority())
	{
		return;
	}

	// 상태가 실제로 변경될 때만 델리게이트 브로드캐스트
	if (bIsActive != bNewActive)
	{
		bIsActive = bNewActive;

		// 반전 적용된 최종 상태 계산
		bool bFinalState = bInvertTrigger ? !bIsActive : bIsActive;

		// 서버에서 트리거 상태 변경 이벤트 브로드캐스트
		OnTriggerStateChanged.Broadcast(bFinalState);

		// 머테리얼 업데이트
		UpdateTriggerMaterial();
	}
}

void ABaseTrigger::OnRep_IsActive()
{
	// 클라이언트에서 리플리케이션을 통해 상태가 변경되었을 때 델리게이트 브로드캐스트
	bool bFinalState = bInvertTrigger ? !bIsActive : bIsActive;
	OnTriggerStateChanged.Broadcast(bFinalState);

	// 머테리얼 업데이트
	UpdateTriggerMaterial();
}

void ABaseTrigger::UpdateTriggerMaterial()
{
	// 머테리얼이 잠겨있으면 업데이트하지 않음
	if (bLockMaterial)
	{
		return;
	}

	// TriggerMesh가 유효하지 않으면 리턴
	if (!TriggerMesh)
	{
		return;
	}

	// 트리거 작동 여부에 따라 머테리얼 선택 (invertTrigger는 출력값만 반전, 시각적 표현과는 무관)
	// bIsActive == true: 트리거가 작동한 상태 -> ActiveMaterial
	// bIsActive == false: 트리거가 작동하지 않은 상태 -> InactiveMaterial
	UMaterialInterface* TargetMaterial = bIsActive ? ActiveMaterial : InactiveMaterial;

	// 머테리얼이 설정되어 있으면 적용
	if (TargetMaterial)
	{
		TriggerMesh->SetOverlayMaterial(TargetMaterial);
	}
	else
	{
		// 머테리얼이 없으면 오버레이 제거
		TriggerMesh->SetOverlayMaterial(nullptr);
	}
}

void ABaseTrigger::LockOverlayMaterial(bool bActive)
{
	// 머테리얼 잠금
	bLockMaterial = true;

	// TriggerMesh가 유효하지 않으면 리턴
	if (!TriggerMesh)
	{
		return;
	}

	// 지정된 상태의 머테리얼로 고정
	UMaterialInterface* TargetMaterial = bActive ? ActiveMaterial : InactiveMaterial;

	// 머테리얼이 설정되어 있으면 적용
	if (TargetMaterial)
	{
		TriggerMesh->SetOverlayMaterial(TargetMaterial);
	}
	else
	{
		// 머테리얼이 없으면 오버레이 제거
		TriggerMesh->SetOverlayMaterial(nullptr);
	}
}

void ABaseTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// bIsActive를 모든 클라이언트에 리플리케이트
	DOREPLIFETIME(ABaseTrigger, bIsActive);
}

