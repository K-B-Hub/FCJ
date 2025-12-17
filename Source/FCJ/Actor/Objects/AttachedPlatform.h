// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AttachedPlatform.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AHoldingObject;

UCLASS()
class FCJ_API AAttachedPlatform : public AActor
{
	GENERATED_BODY()

public:
	AAttachedPlatform();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* AttachTriggerBox;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttachedPlatform", meta = (DisplayName = "부착 오프셋", Tooltip = "HoldingObject가 플랫폼에 부착될 때 적용할 오프셋"))
	FVector AttachOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttachedPlatform", meta = (DisplayName = "부착 시 물리 비활성화", Tooltip = "true면 부착 시 HoldingObject의 물리를 비활성화합니다"))
	bool bDisablePhysicsOnAttach;

protected:
	// Overlap detection
	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Server RPC for attachment
	UFUNCTION(Server, Reliable)
	void ServerAttachObject(AHoldingObject* ObjectToAttach);

	// Multicast for visual sync
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttachObject(AHoldingObject* ObjectToAttach);

	// Replicated state
	UPROPERTY(Replicated)
	AHoldingObject* AttachedObject;

private:
	void AttachObjectToPlatform(AHoldingObject* ObjectToAttach);
};
