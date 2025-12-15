// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ContinuousFadingPlatform.generated.h"

class AFadingPlatform;
class UBoxComponent;

/**
 * AContinuousFadingPlatform
 *
 * Continuous platform system for sequential platforming challenges.
 * When a player steps on this platform, it activates all platforms within its trigger box,
 * then fades itself after a delay, and respawns after another delay.
 *
 * Architecture:
 * - Contains FadingPlatform as ChildActorComponent
 * - TriggerBox (UBoxComponent) detects overlapping ContinuousFadingPlatform actors
 * - Delegate-driven: Binds to FadingPlatform's OnPlatformStepped event
 * - Timer-based: No Tick overhead
 * - Server-authoritative: All state changes handled on server
 *
 * Gameplay Flow:
 * 1. Player steps on FadingPlatform -> OnPlatformStepped delegate fires
 * 2. Activates all ContinuousFadingPlatform within TriggerBox range
 * 3. After FadeDelay, current platform fades out
 * 4. After RespawnDelay, current platform respawns
 *
 * Use Case:
 * Create sequential platforming puzzles where players must quickly move to the next
 * platform before the current one disappears.
 */
UCLASS()
class FCJ_API AContinuousFadingPlatform : public AActor
{
	GENERATED_BODY()

public:
	AContinuousFadingPlatform();

protected:
	virtual void BeginPlay() override;

public:
	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

	/**
	 * Resets the platform to its initial state.
	 * Clears all timers and resets FadingPlatform state.
	 * Server-only function.
	 */
	UFUNCTION(BlueprintCallable, Category = "Continuous Platform")
	void ResetPlatform();

protected:
	/**
	 * Called when player steps on the FadingPlatform.
	 * Activates next platforms and starts fade timer.
	 */
	UFUNCTION()
	void OnPlatformStepped(AFadingPlatform* SteppedPlatform);

	/**
	 * Activates all ContinuousFadingPlatform actors within TriggerBox range.
	 * Server-only function.
	 */
	void ActivateNextPlatforms();

	/**
	 * Fades the current platform (makes it invisible and disables collision).
	 * Server-only function.
	 */
	void FadeCurrentPlatform();

	/**
	 * Respawns the current platform (makes it visible and enables collision).
	 * Server-only function.
	 */
	void RespawnCurrentPlatform();

protected:
	/** The FadingPlatform that players step on */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UChildActorComponent> PlatformChild;

	/** TriggerBox for detecting next platforms to activate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Time delay before fading current platform after being stepped on (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Continuous Platform", meta = (ClampMin = "0.0", ToolTip = "발판을 밟은 후 현재 발판이 사라지기까지의 시간(초)"))
	float FadeDelay = 2.0f;

	/** Time delay before respawning platform after it fades (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Continuous Platform", meta = (ClampMin = "0.0", ToolTip = "발판이 사라진 후 다시 나타나기까지의 시간(초)"))
	float RespawnDelay = 3.0f;

	/** Whether the platform starts in active state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Continuous Platform", meta = (ToolTip = "시작 시 발판이 활성화된 상태인지 여부"))
	bool bStartActive = true;

	/** Whether the platform is currently active (visible and has collision) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Continuous Platform")
	bool bIsActive = true;

private:
	/** Timer handle for fading the current platform */
	FTimerHandle FadeTimerHandle;

	/** Timer handle for respawning the platform */
	FTimerHandle RespawnTimerHandle;

	/** Cached reference to the FadingPlatform actor */
	UPROPERTY()
	TObjectPtr<AFadingPlatform> FadingPlatform;

	/** Cached reference to the next platform (found in BeginPlay) */
	UPROPERTY()
	TObjectPtr<AContinuousFadingPlatform> NextPlatform;
};
