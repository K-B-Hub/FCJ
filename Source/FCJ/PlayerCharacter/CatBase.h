// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "CatBase.generated.h"

class AWallJumpObject;

UCLASS(BlueprintType, Blueprintable)
class FCJ_API ACatBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACatBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Camera components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	// 특수 행동 감지 영역 (BiteCat: 잡기, AttackCat: 공격/패링)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Special Action", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* SpecialActionBox;

	// 파쿠르 하단 감지 박스 (스태틱메시와 오버랩되어야 함)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* ParkourLowerBox;

	// 파쿠르 상단 감지 박스 (스태틱메시와 오버랩되면 안됨)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* ParkourUpperBox;


	// Blueprint configurable properties
	// 카메라 암 길이 (캐릭터로부터 카메라까지의 거리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "캐릭터로부터 카메라까지의 거리를 설정합니다"))
	float SpringArmLength = 300.0f;

	// 카메라 암의 오프셋 (캐릭터 중심점으로부터의 상대 위치)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "캐릭터 중심점으로부터 카메라 암의 상대적 위치를 설정합니다"))
	FVector SpringArmOffset = FVector(0.0f, 0.0f, 60.0f);

	// 카메라 암의 기본 피치 각도 (위아래 각도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "카메라의 기본 상하 각도를 설정합니다 (음수는 위쪽)"))
	float SpringArmPitch = -20.0f;

	// 폰 컨트롤러 회전 사용 여부 (마우스 입력으로 카메라 회전 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "마우스 입력으로 카메라를 회전시킬 수 있는지 설정합니다"))
	bool bUsePawnControlRotation = true;

	// 피치 회전 상속 (캐릭터의 상하 회전을 카메라가 따라갈지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "캐릭터의 상하 회전을 카메라가 따라갈지 설정합니다"))
	bool bInheritPitch = true;

	// 요 회전 상속 (캐릭터의 좌우 회전을 카메라가 따라갈지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "캐릭터의 좌우 회전을 카메라가 따라갈지 설정합니다"))
	bool bInheritYaw = true;

	// 롤 회전 상속 (캐릭터의 기울임을 카메라가 따라갈지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "캐릭터의 기울임을 카메라가 따라갈지 설정합니다"))
	bool bInheritRoll = false;

	// 캐릭터의 지상 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings", meta = (ToolTip = "캐릭터가 지상에서 이동하는 최대 속도를 설정합니다"))
	float MovementSpeed = 600.0f;

	// 점프 시 초기 수직 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings", meta = (ToolTip = "점프할 때 캐릭터가 받는 초기 상승 속도를 설정합니다"))
	float JumpVelocity = 420.0f;
	
	UPROPERTY(ReplicatedUsing = OnRep_SpeedModifier, BlueprintReadOnly, Category = "Movement")
	float CurrentSpeedModifier = 1.0f;
	
	// 공중에서의 이동 제어력 (0~1 사이값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings", meta = (ToolTip = "공중에 있을 때 방향 전환이 얼마나 쉬운지 설정합니다 (0=불가능, 1=지상과 동일)"))
	float AirControl = 0.5f;

	// Wall Jump Settings
	// 벽점프 감지 반경 (이 거리 내의 벽만 감지됩니다)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump Settings", meta = (ToolTip = "벽점프가 가능한 최대 거리를 설정합니다"))
	float WallJumpDetectionRadius = 150.0f;

	// 벽점프 쿨다운 시간 (연속 벽점프 방지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump Settings", meta = (ToolTip = "벽점프 후 다음 벽점프까지 기다려야 하는 시간(초)을 설정합니다"))
	float WallJumpCooldown = 0.5f;

	// 공중에서 가능한 최대 벽점프 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump Settings", meta = (ToolTip = "착지하기 전까지 연속으로 할 수 있는 벽점프 횟수를 설정합니다"))
	int32 MaxWallJumpsInAir = 1;

	// Special Action Settings
	// 특수 행동 감지 영역의 크기 (BiteCat: 잡기 범위, AttackCat: 공격/패링 범위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Action Settings", meta = (ToolTip = "특수 행동(잡기/공격)이 가능한 박스 영역의 크기를 설정합니다"))
	FVector SpecialActionBoxExtent = FVector(100.0f, 50.0f, 50.0f);

	// Parkour Settings
	// 파쿠르 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour Settings", meta = (ToolTip = "파쿠르 시 재생할 루트모션 애니메이션 몽타주"))
	class UAnimMontage* ParkourMontage;

private:
	// Wall Jump variables
	float LastWallJumpTime = 0.0f;
	int32 CurrentWallJumpsInAir = 0;
	bool bCanWallJump = true;

	// Parkour variables (replicated for network sync)
	UPROPERTY()
	bool bTryingToParkour = false;
	UPROPERTY(Replicated)
	bool bIsPerformingParkour = false;
	UPROPERTY(Replicated)
	AActor* CurrentParkourActor = nullptr;

	// Montage playing state (replicated for network sync)
	UPROPERTY(Replicated)
	bool bIsMontageePlaying = false;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Wall Jump functions
	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	AWallJumpObject* FindNearestWallJumpObject() const;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	bool CanPerformWallJump() const;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	void PerformWallJump();

	// Server RPC for wall jump
	UFUNCTION(Server, Reliable, Category = "Wall Jump")
	void ServerPerformWallJump(FVector JumpDirection);

	// Multicast RPC for wall jump effects
	UFUNCTION(NetMulticast, Reliable, Category = "Wall Jump")
	void MulticastPerformWallJump(FVector JumpDirection);

	// Override Jump to include wall jump and climbing
	virtual void Jump() override;

	virtual void StopJumping() override;

	// Parkour functions
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	AActor* DetectParkourTarget() const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	bool CanPerformParkour() const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	void PerformParkour();

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	bool IsPerformingParkour() const { return bIsPerformingParkour; }

	// Server RPC for parkour
	UFUNCTION(Server, Reliable, Category = "Parkour")
	void ServerPerformParkour(AActor* ParkourTarget);

	// Multicast RPC for parkour effects
	UFUNCTION(NetMulticast, Reliable, Category = "Parkour")
	void MulticastPerformParkour(AActor* ParkourTarget);

	// Parkour helper functions
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector CalculateParkourStartLocation(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector CalculateParkourTargetLocation(AActor* Actor) const;

	// Precise parkour positioning functions
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector CalculateParkourStartLocationPrecise(AActor* Actor) const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector CalculateParkourTargetLocationPrecise(AActor* Actor) const;

	// Parkour completion callback
	void OnParkourMontageCompleted();

	// Montage state management
	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsPlayingMontage() const { return bIsMontageePlaying; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetMontageePlaying(bool InPlaying) { bIsMontageePlaying = InPlaying; }

	// Public getters for components
	UFUNCTION(BlueprintCallable, Category = "Camera")
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	UFUNCTION(BlueprintCallable, Category = "Special Action")
	UBoxComponent* GetSpecialActionBox() const { return SpecialActionBox; }

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	UBoxComponent* GetParkourLowerBox() const { return ParkourLowerBox; }

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	UBoxComponent* GetParkourUpperBox() const { return ParkourUpperBox; }


	// Virtual functions for derived classes to override
	UFUNCTION(BlueprintImplementableEvent, Category = "Special Actions")
	void OnSpecialAction();

	UFUNCTION(BlueprintCallable, Category = "Special Actions")
	virtual void PerformSpecialAction() { OnSpecialAction(); }

	//외부에서 속도 제어할 시 접근할 함수
	UFUNCTION(BlueprintCallable, Category = "Speed Control")
	void ApplySpeedModifier(float Multiplier);
	
private:
	// Apply Blueprint settings to components
	void ApplyBlueprintSettings();

	//AppplySpeedModifier에서 호출되는 RPC
	UFUNCTION(Server, Reliable, Category = "Speed Control")
	void ServerSetMovementSpeed(float Multiplier);

	//서버에서 바꾸는 SpeedModifier의 RepNotify
	UFUNCTION(Category = "Speed Control")
	void OnRep_SpeedModifier();

	//실제 속도 업데이트
	void UpdateMovementSpeed(float Multiplier);
};
