// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FadingPlatform.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

// 발판이 밟혔을 때 호출되는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformStepped, AFadingPlatform*, SteppedPlatform);

/**
 * 플레이어가 올라오면 일정 시간 후 사라지는 발판
 * Blueprint에서 사라지는 시간과 재생성 시간을 설정할 수 있습니다.
 */
UCLASS()
class FCJ_API AFadingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AFadingPlatform();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 발판 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PlatformMesh;

	// 플레이어 감지용 충돌 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	// 캐릭터가 올라온 후 사라지기까지의 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fading Platform", meta = (ClampMin = "0.0", ToolTip = "플레이어가 올라온 후 발판이 사라지기까지의 시간(초)"))
	float FadeDelay;

	// 사라진 후 다시 나타나기까지의 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fading Platform", meta = (ClampMin = "0.0", ToolTip = "발판이 사라진 후 다시 나타나기까지의 시간(초)"))
	float RespawnDelay;

	// 자동으로 다시 나타날지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fading Platform", meta = (ToolTip = "발판이 사라진 후 자동으로 다시 나타날지 여부"))
	bool bAutoRespawn;

	// 현재 발판이 활성화되어 있는지 여부 (네트워크 리플리케이션)
	UPROPERTY(ReplicatedUsing = OnRep_IsActive, BlueprintReadOnly, Category = "Fading Platform")
	bool bIsActive;

	// 발판이 트리거되었는지 여부 (타이머 중복 방지)
	bool bIsTriggered;

	// 트리거가 활성화되어 있는지 여부 (PairFadingPlatform에서 사용)
	bool bTriggerEnabled;

	// 타이머 핸들
	FTimerHandle FadeTimerHandle;
	FTimerHandle RespawnTimerHandle;

	/**
	 * 플레이어가 발판에 올라왔을 때 호출되는 함수
	 */
	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 발판을 사라지게 하는 함수 (서버에서만 호출)
	 */
	UFUNCTION()
	void FadePlatform();

	/**
	 * 발판을 다시 나타나게 하는 함수 (서버에서만 호출)
	 */
	UFUNCTION()
	void RespawnPlatform();

	/**
	 * bIsActive 리플리케이션 콜백 - 모든 클라이언트에서 시각적 상태 업데이트
	 */
	UFUNCTION()
	void OnRep_IsActive();

	/**
	 * 발판의 시각적 상태를 업데이트 (활성화/비활성화)
	 */
	void UpdateVisualState();

public:
	// 발판이 밟혔을 때 브로드캐스트되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Fading Platform")
	FOnPlatformStepped OnPlatformStepped;

	/**
	 * 발판이 현재 활성화되어 있는지 확인
	 */
	UFUNCTION(BlueprintCallable, Category = "Fading Platform")
	bool IsActive() const { return bIsActive; }

	/**
	 * 발판을 강제로 리셋 (Blueprint에서 호출 가능)
	 */
	UFUNCTION(BlueprintCallable, Category = "Fading Platform")
	void ResetPlatform();

	/**
	 * 트리거 활성화/비활성화 설정
	 */
	UFUNCTION(BlueprintCallable, Category = "Fading Platform")
	void SetTriggerEnabled(bool bEnabled);

	/**
	 * 발판을 보이게 하지만 트리거는 비활성화 상태로 활성화
	 * PairFadingPlatform에서 사용
	 */
	UFUNCTION(BlueprintCallable, Category = "Fading Platform")
	void ActivatePlatformWithoutTrigger();
};
