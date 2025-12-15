// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Objects/ContinuousFadingPlatform.h"
#include "Actor/Objects/FadingPlatform.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"

AContinuousFadingPlatform::AContinuousFadingPlatform()
{
	// Tick 비활성화 (타이머 기반 이벤트 처리 사용)
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 리플리케이션 활성화
	bReplicates = true;

	// 루트 컴포넌트 생성 (빈 SceneComponent)
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	// FadingPlatform ChildActorComponent 생성
	PlatformChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("PlatformChild"));
	PlatformChild->SetupAttachment(RootComponent);
	PlatformChild->SetChildActorClass(AFadingPlatform::StaticClass());

	// TriggerBox 생성 (다음 플랫폼 감지용)
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f)); // 기본 범위 설정

	// TriggerBox 충돌 설정 (다른 ContinuousFadingPlatform 감지용)
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	// 기본값 설정
	FadeDelay = 2.0f;
	RespawnDelay = 3.0f;
	bStartActive = true;
	bIsActive = true;
}

void AContinuousFadingPlatform::BeginPlay()
{
	Super::BeginPlay();

	// FadingPlatform 참조 캐싱
	if (PlatformChild)
	{
		FadingPlatform = Cast<AFadingPlatform>(PlatformChild->GetChildActor());

		if (FadingPlatform && HasAuthority())
		{
			// 델리게이트 바인딩
			FadingPlatform->OnPlatformStepped.AddDynamic(this, &AContinuousFadingPlatform::OnPlatformStepped);
			UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Delegate bound for platform: %s"), *GetName());

			// 시작 상태 설정
			if (!bStartActive)
			{
				bIsActive = false;
				FadingPlatform->ResetPlatform();
				FadingPlatform->SetActorHiddenInGame(true);
				FadingPlatform->SetActorEnableCollision(false);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ContinuousFadingPlatform] PlatformChild is NULL for: %s"), *GetName());
	}

	// TriggerBox와 겹치는 다음 플랫폼 찾기 (직접 오버랩 쿼리 사용)
	if (TriggerBox && HasAuthority())
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		bool bOverlap = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			TriggerBox->GetComponentLocation(),
			TriggerBox->GetComponentQuat(),
			ECC_WorldStatic,
			FCollisionShape::MakeBox(TriggerBox->GetScaledBoxExtent()),
			QueryParams
		);

		if (bOverlap)
		{
			UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Direct overlap query found %d results"), OverlapResults.Num());

			for (const FOverlapResult& Result : OverlapResults)
			{
				AActor* OverlappedActor = Result.GetActor();
				if (!OverlappedActor || OverlappedActor == this)
				{
					continue;
				}

				UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Checking overlapped actor: %s"), *OverlappedActor->GetName());

				// 직접 ContinuousFadingPlatform인 경우
				AContinuousFadingPlatform* Platform = Cast<AContinuousFadingPlatform>(OverlappedActor);

				// ChildActor인 경우, Owner 체인을 따라 올라가기
				if (!Platform)
				{
					// 1단계: 컴포넌트의 Owner (보통 FadingPlatform)
					AActor* ComponentOwner = Result.GetComponent() ? Result.GetComponent()->GetOwner() : nullptr;
					if (ComponentOwner)
					{
						UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Component owner (level 1): %s"), *ComponentOwner->GetName());

						// ComponentOwner가 ContinuousFadingPlatform인지 확인
						Platform = Cast<AContinuousFadingPlatform>(ComponentOwner);

						// 2단계: ComponentOwner의 Owner (ContinuousFadingPlatform)
						if (!Platform)
						{
							AActor* ParentOwner = ComponentOwner->GetAttachParentActor();
							if (ParentOwner)
							{
								UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Parent owner (level 2): %s"), *ParentOwner->GetName());
								Platform = Cast<AContinuousFadingPlatform>(ParentOwner);

								if (Platform)
								{
									UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Found ContinuousFadingPlatform at level 2!"));
								}
							}
							else
							{
								UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Parent owner is nullptr"));
							}
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Found ContinuousFadingPlatform at level 1!"));
						}
					}
				}

				if (Platform && Platform != this)
				{
					NextPlatform = Platform;
					UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Found next platform via direct query: %s"), *NextPlatform->GetName());
					break; // 첫 번째 플랫폼만 사용
				}
			}
		}

		if (NextPlatform)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Successfully cached next platform: %s -> %s"),
				*GetName(), *NextPlatform->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] No next platform found for: %s"), *GetName());
		}
	}
}

void AContinuousFadingPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AContinuousFadingPlatform, bIsActive);
}

void AContinuousFadingPlatform::OnPlatformStepped(AFadingPlatform* SteppedPlatform)
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Platform stepped: %s"), *GetName());

	// 다음 플랫폼들 활성화
	ActivateNextPlatforms();

	// FadeDelay 후 현재 플랫폼 사라지기
	GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AContinuousFadingPlatform::FadeCurrentPlatform, FadeDelay, false);
}

void AContinuousFadingPlatform::ActivateNextPlatforms()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	// 캐싱된 다음 플랫폼이 없으면 리턴
	if (!NextPlatform)
	{
		UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] No next platform to activate for: %s"), *GetName());
		return;
	}

	// 다음 플랫폼의 FadingPlatform 활성화
	if (NextPlatform->FadingPlatform)
	{
		NextPlatform->bIsActive = true;
		NextPlatform->FadingPlatform->ResetPlatform();
		NextPlatform->FadingPlatform->SetActorHiddenInGame(false);
		NextPlatform->FadingPlatform->SetActorEnableCollision(true);

		// 진행 중인 타이머가 있다면 정리
		NextPlatform->GetWorldTimerManager().ClearTimer(NextPlatform->FadeTimerHandle);
		NextPlatform->GetWorldTimerManager().ClearTimer(NextPlatform->RespawnTimerHandle);

		UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Activated next platform: %s -> %s"),
			*GetName(), *NextPlatform->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] Next platform's FadingPlatform is NULL: %s"),
			*NextPlatform->GetName());
	}
}

void AContinuousFadingPlatform::FadeCurrentPlatform()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	if (!FadingPlatform)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] FadingPlatform is NULL for: %s"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Fading platform: %s"), *GetName());

	// FadingPlatform의 자체 FadePlatform 호출
	// 이렇게 하면 FadingPlatform의 네트워크 리플리케이션이 제대로 작동함
	bIsActive = false;
	FadingPlatform->SetActorHiddenInGame(true);
	FadingPlatform->SetActorEnableCollision(false);

	// RespawnDelay 후 다시 나타나기
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AContinuousFadingPlatform::RespawnCurrentPlatform, RespawnDelay, false);
}

void AContinuousFadingPlatform::RespawnCurrentPlatform()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	if (!FadingPlatform)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] FadingPlatform is NULL for: %s"), *GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Respawning platform: %s"), *GetName());

	// FadingPlatform 다시 활성화
	bIsActive = true;
	FadingPlatform->ResetPlatform();
	FadingPlatform->SetActorHiddenInGame(false);
	FadingPlatform->SetActorEnableCollision(true);

	// 타이머 정리
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

void AContinuousFadingPlatform::ResetPlatform()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContinuousFadingPlatform] ResetPlatform called on client - ignored"));
		return;
	}

	// 모든 타이머 정리
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	// FadingPlatform 리셋
	if (FadingPlatform)
	{
		FadingPlatform->ResetPlatform();
		FadingPlatform->SetActorHiddenInGame(false);
		FadingPlatform->SetActorEnableCollision(true);
	}

	// 초기 상태로 리셋
	bIsActive = true;

	UE_LOG(LogTemp, Log, TEXT("[ContinuousFadingPlatform] Platform reset: %s"), *GetName());
}

