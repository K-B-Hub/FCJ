// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PairFadingPlatform.generated.h"

class UChildActorComponent;
class AFadingPlatform;

/**
 * 두 개의 FadingPlatform을 번갈아가며 활성화하는 발판 시스템
 * 한 발판을 밟으면 다른 발판이 나타나고, 밟힌 발판은 1초 후 사라집니다.
 * 새로 나타난 발판은 이전 발판이 사라진 후(1초 후)에 밟힌 여부를 판단합니다.
 */
UCLASS()
class FCJ_API APairFadingPlatform : public AActor
{
	GENERATED_BODY()

public:
	APairFadingPlatform();

protected:
	virtual void BeginPlay() override;

	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	// 첫 번째 발판 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (ToolTip = "첫 번째 FadingPlatform - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* FirstPlatformComponent;

	// 두 번째 발판 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (ToolTip = "두 번째 FadingPlatform - Blueprint에서 Transform 조절 가능"))
	UChildActorComponent* SecondPlatformComponent;

	// 처음 시작할 때 첫 번째 발판을 활성화할지 여부 (false면 두 번째 발판이 활성화)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pair Fading Platform", meta = (ToolTip = "true: 첫 번째 발판부터 시작, false: 두 번째 발판부터 시작"))
	bool bStartWithFirstPlatform;

	// 발판이 사라진 후 다른 발판의 트리거를 활성화하는 딜레이 (기본 1초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pair Fading Platform", meta = (ClampMin = "0.0", ToolTip = "발판이 사라진 후 다른 발판의 트리거를 활성화하는 시간(초)"))
	float TriggerEnableDelay;

private:
	// 캐시된 발판 액터들
	UPROPERTY()
	AFadingPlatform* FirstPlatform;

	UPROPERTY()
	AFadingPlatform* SecondPlatform;

	// 타이머 핸들
	FTimerHandle EnableTriggerTimerHandle;

	/**
	 * 첫 번째 발판이 밟혔을 때 호출되는 콜백
	 */
	UFUNCTION()
	void OnFirstPlatformStepped(AFadingPlatform* SteppedPlatform);

	/**
	 * 두 번째 발판이 밟혔을 때 호출되는 콜백
	 */
	UFUNCTION()
	void OnSecondPlatformStepped(AFadingPlatform* SteppedPlatform);

	/**
	 * 다른 발판의 트리거를 활성화하는 함수 (타이머에서 호출)
	 */
	void EnableOtherPlatformTrigger(AFadingPlatform* PlatformToEnable);

public:
	/**
	 * 첫 번째 발판 액터를 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "Pair Fading Platform")
	AFadingPlatform* GetFirstPlatform() const { return FirstPlatform; }

	/**
	 * 두 번째 발판 액터를 반환
	 */
	UFUNCTION(BlueprintCallable, Category = "Pair Fading Platform")
	AFadingPlatform* GetSecondPlatform() const { return SecondPlatform; }
};
