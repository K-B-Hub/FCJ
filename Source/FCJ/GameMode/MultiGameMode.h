// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FCJ_API AMultiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMultiGameMode();

protected:
	// Blueprint configurable player controller class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class APlayerController> MultiPlayerControllerClass;

	// Blueprint configurable default pawn class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class APawn> DefaultPawnClass_Multi;

protected:
	virtual void BeginPlay() override;

public:
	// Override to set custom classes
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable)
	void HostLANGame();

	UFUNCTION(BlueprintCallable)
	void JoinLANGame();

	// 플레이어 역할 교체
	UFUNCTION(BlueprintCallable)
	void SwapPlayerRoles();

	// 플레이어 역할 가져오기 (0 = Player1, 1 = Player2)
	UFUNCTION(BlueprintCallable)
	int32 GetPlayerRole(APlayerController* PC) const;

protected:
	// 캐릭터 클래스들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Classes")
	TSubclassOf<class AAttackCat> AttackCatClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Classes")
	TSubclassOf<class ABiteCat> BiteCatClass;

private:
	// Apply saved display settings
	void ApplySavedDisplaySettings();

	// 플레이어 역할 매핑 (PlayerController -> Role Index)
	// 0 = 1P (AttackCat), 1 = 2P (BiteCat)
	TMap<APlayerController*, int32> PlayerRoles;

	// 플레이어가 접속할 때 역할 할당 및 캐릭터 생성
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Subsystem에서 저장된 역할 정보 가져오기
	int32 GetRoleFromSubsystem(APlayerController* PC);
}; 
