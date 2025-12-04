// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Objects/PairFadingPlatform.h"
#include "Actor/Objects/FadingPlatform.h"
#include "Components/ChildActorComponent.h"
#include "TimerManager.h"

APairFadingPlatform::APairFadingPlatform()
{
	// Tick 비활성화 (델리게이트 및 타이머 기반 이벤트 처리 사용)
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 리플리케이션 활성화
	bReplicates = true;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// 첫 번째 발판 ChildActorComponent 생성
	FirstPlatformComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("FirstPlatformComponent"));
	FirstPlatformComponent->SetupAttachment(RootSceneComponent);
	FirstPlatformComponent->SetChildActorClass(AFadingPlatform::StaticClass());

	// 두 번째 발판 ChildActorComponent 생성
	SecondPlatformComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("SecondPlatformComponent"));
	SecondPlatformComponent->SetupAttachment(RootSceneComponent);
	SecondPlatformComponent->SetChildActorClass(AFadingPlatform::StaticClass());

	// 기본값 설정
	bStartWithFirstPlatform = true;
	TriggerEnableDelay = 1.0f;

	FirstPlatform = nullptr;
	SecondPlatform = nullptr;
}

void APairFadingPlatform::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 초기화 로직 실행
	if (!HasAuthority())
	{
		return;
	}

	// 자식 액터들 캐시
	if (FirstPlatformComponent)
	{
		FirstPlatform = Cast<AFadingPlatform>(FirstPlatformComponent->GetChildActor());
	}

	if (SecondPlatformComponent)
	{
		SecondPlatform = Cast<AFadingPlatform>(SecondPlatformComponent->GetChildActor());
	}

	// 발판이 없으면 경고 로그
	if (!FirstPlatform || !SecondPlatform)
	{
		UE_LOG(LogTemp, Error, TEXT("[PairFadingPlatform] One or both platforms are missing! FirstPlatform: %s, SecondPlatform: %s"),
			FirstPlatform ? TEXT("Valid") : TEXT("NULL"),
			SecondPlatform ? TEXT("Valid") : TEXT("NULL"));
		return;
	}

	// 델리게이트 바인딩
	FirstPlatform->OnPlatformStepped.AddDynamic(this, &APairFadingPlatform::OnFirstPlatformStepped);
	SecondPlatform->OnPlatformStepped.AddDynamic(this, &APairFadingPlatform::OnSecondPlatformStepped);

	// 초기 상태 설정
	if (bStartWithFirstPlatform)
	{
		// 첫 번째 발판 활성화, 두 번째 발판 비활성화
		FirstPlatform->ResetPlatform();
		FirstPlatform->SetTriggerEnabled(true);

		SecondPlatform->ActivatePlatformWithoutTrigger();
		SecondPlatform->SetActorHiddenInGame(true);
		SecondPlatform->SetActorEnableCollision(false);

		UE_LOG(LogTemp, Log, TEXT("[PairFadingPlatform] Started with FirstPlatform active"));
	}
	else
	{
		// 두 번째 발판 활성화, 첫 번째 발판 비활성화
		SecondPlatform->ResetPlatform();
		SecondPlatform->SetTriggerEnabled(true);

		FirstPlatform->ActivatePlatformWithoutTrigger();
		FirstPlatform->SetActorHiddenInGame(true);
		FirstPlatform->SetActorEnableCollision(false);

		UE_LOG(LogTemp, Log, TEXT("[PairFadingPlatform] Started with SecondPlatform active"));
	}
}

void APairFadingPlatform::OnFirstPlatformStepped(AFadingPlatform* SteppedPlatform)
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PairFadingPlatform] FirstPlatform stepped - Activating SecondPlatform"));

	// 두 번째 발판을 즉시 보이게 하지만 트리거는 비활성화
	if (SecondPlatform)
	{
		SecondPlatform->ActivatePlatformWithoutTrigger();
		SecondPlatform->SetActorHiddenInGame(false);
		SecondPlatform->SetActorEnableCollision(true);

		// TriggerEnableDelay 후에 두 번째 발판의 트리거 활성화
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &APairFadingPlatform::EnableOtherPlatformTrigger, SecondPlatform);
		GetWorldTimerManager().SetTimer(EnableTriggerTimerHandle, TimerDelegate, TriggerEnableDelay, false);
	}
}

void APairFadingPlatform::OnSecondPlatformStepped(AFadingPlatform* SteppedPlatform)
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PairFadingPlatform] SecondPlatform stepped - Activating FirstPlatform"));

	// 첫 번째 발판을 즉시 보이게 하지만 트리거는 비활성화
	if (FirstPlatform)
	{
		FirstPlatform->ActivatePlatformWithoutTrigger();
		FirstPlatform->SetActorHiddenInGame(false);
		FirstPlatform->SetActorEnableCollision(true);

		// TriggerEnableDelay 후에 첫 번째 발판의 트리거 활성화
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &APairFadingPlatform::EnableOtherPlatformTrigger, FirstPlatform);
		GetWorldTimerManager().SetTimer(EnableTriggerTimerHandle, TimerDelegate, TriggerEnableDelay, false);
	}
}

void APairFadingPlatform::EnableOtherPlatformTrigger(AFadingPlatform* PlatformToEnable)
{
	if (!HasAuthority() || !PlatformToEnable)
	{
		return;
	}

	// 발판의 트리거 활성화
	PlatformToEnable->SetTriggerEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("[PairFadingPlatform] Enabled trigger for platform: %s"), *PlatformToEnable->GetName());
}
