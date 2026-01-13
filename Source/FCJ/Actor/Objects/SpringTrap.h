// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "SpringTrap.generated.h"

class ACatBase;

/**
 * 함정 상태를 나타내는 열거형
 */
UENUM(BlueprintType)
enum class ETrapState : uint8
{
	Retracted    UMETA(DisplayName = "Retracted"),      // 수축된 상태 (기본)
	Extending    UMETA(DisplayName = "Extending"),      // 확장 중
	Extended     UMETA(DisplayName = "Extended"),       // 완전히 확장됨
	Retracting   UMETA(DisplayName = "Retracting"),     // 수축 중
	Cooldown     UMETA(DisplayName = "Cooldown")        // 쿨다운 중
};

/**
 * 캐릭터가 밟으면 바닥에서 메시가 비스듬하게 튀어나와 캐릭터를 날려버리는 함정
 * 재사용 가능하며, 일정 시간 후 다시 수축합니다.
 */
UCLASS()
class FCJ_API ASpringTrap : public AActor
{
	GENERATED_BODY()

public:
	ASpringTrap();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ===== 컴포넌트 =====

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TrapMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	// ===== 발사 설정 =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Launch Settings",
		meta = (ClampMin = "0.0", ClampMax = "90.0", ToolTip = "발사 각도 (도 단위, 0 = 수평, 90 = 수직)"))
	float LaunchAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Launch Settings",
		meta = (ClampMin = "100.0", ToolTip = "발사 힘의 크기 (수평 방향 기준)"))
	float LaunchForce = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Launch Settings",
		meta = (ClampMin = "0.0", ToolTip = "발사 힘의 수직 성분 크기"))
	float VerticalLaunchForce = 800.0f;

	// ===== 이동 설정 =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Movement Settings",
		meta = (ClampMin = "10.0", ToolTip = "트랩 메시가 확장되는 높이 (로컬 Z축 오프셋, 단위)"))
	float ExtensionHeight = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Movement Settings",
		meta = (ClampMin = "0.1", ToolTip = "트랩 메시가 확장되는 속도 (단위/초)"))
	float ExtensionSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Movement Settings",
		meta = (ClampMin = "0.1", ToolTip = "트랩 메시가 수축되는 속도 (단위/초)"))
	float RetractionSpeed = 300.0f;

	// ===== 타이밍 설정 =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Timing Settings",
		meta = (ClampMin = "0.0", ToolTip = "완전히 확장된 후 수축 시작까지 대기 시간 (초)"))
	float ExtendedHoldTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Timing Settings",
		meta = (ClampMin = "0.0", ToolTip = "수축 완료 후 다음 트리거까지 대기 시간 (초)"))
	float CooldownTime = 2.0f;

	// ===== 시각 설정 =====

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Visual Settings",
		meta = (ToolTip = "트리거 박스의 크기 (감지 영역)"))
	FVector TriggerBoxExtent = FVector(100.0f, 100.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Trap - Visual Settings",
		meta = (ToolTip = "트리거 박스의 로컬 오프셋 (트랩 위치 기준)"))
	FVector TriggerBoxOffset = FVector(0.0f, 0.0f, 75.0f);

	// ===== 복제된 상태 =====

	UPROPERTY(ReplicatedUsing = OnRep_TrapState, BlueprintReadOnly, Category = "Spring Trap State")
	ETrapState CurrentState;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Spring Trap State")
	bool bIsExtended;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Spring Trap State")
	bool bCanTrigger;

	// ===== Overlap 감지 =====

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// ===== 상태 전환 함수 (서버 전용) =====

	void ActivateTrap();
	void OnExtensionComplete();
	void StartRetraction();
	void OnRetractionComplete();
	void OnCooldownComplete();

	// ===== 발사 메커니즘 =====

	void LaunchCharacter(ACatBase* Character);
	FVector CalculateLaunchDirection() const;

	// ===== 애니메이션 =====

	void UpdateTrapMeshPosition(float DeltaTime);

	// ===== 네트워크 =====

	UFUNCTION()
	void OnRep_TrapState();

private:
	// 초기 및 확장된 위치
	FVector InitialTrapLocation;
	FVector ExtendedTrapLocation;

	// 타이머 핸들
	FTimerHandle ExtensionTimerHandle;
	FTimerHandle RetractTimerHandle;
	FTimerHandle RetractionTimerHandle;
	FTimerHandle CooldownTimerHandle;

public:
	virtual void Tick(float DeltaTime) override;

	// ===== Blueprint 조회 함수 =====

	UFUNCTION(BlueprintCallable, Category = "Spring Trap")
	ETrapState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Spring Trap")
	bool CanTrigger() const { return bCanTrigger; }

	UFUNCTION(BlueprintCallable, Category = "Spring Trap")
	void ResetTrap();
};
