// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnVolume.generated.h"

class UBoxComponent;
class UCapsuleComponent;
class USceneComponent;

UCLASS()
class FCJ_API ARespawnVolume : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARespawnVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

private:
	// 루트 씬 컴포넌트 (독립적인 컴포넌트 이동을 위함)
	UPROPERTY(VisibleAnywhere, Category = "Respawn")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, Category = "Respawn")
	UBoxComponent* RespawnBox;

	// 리스폰 위치 시각화용 캡슐 (에디터에서만 보임, 게임에서는 숨김)
	// RootComponent에 독립적으로 attach되어 위치 조절 용이
	UPROPERTY(VisibleAnywhere, Category = "Respawn")
	UCapsuleComponent* SpawnPointVisualizer;

};
