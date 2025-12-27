// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Volumes/ObjectSlowFallVolume.h"
#include "Actor/Objects/HoldingObject.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AObjectSlowFallVolume::AObjectSlowFallVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SlowFallBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SlowFallBox"));
	RootComponent = SlowFallBox;
	SlowFallBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SlowFallBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	SlowFallBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SlowFallBox->SetGenerateOverlapEvents(true);
	SlowFallBox->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
}

// Called when the game starts or when spawned
void AObjectSlowFallVolume::BeginPlay()
{
	Super::BeginPlay();

	SlowFallBox->OnComponentBeginOverlap.AddDynamic(this, &AObjectSlowFallVolume::OnOverlapBegin);
	SlowFallBox->OnComponentEndOverlap.AddDynamic(this, &AObjectSlowFallVolume::OnOverlapEnd);
}

void AObjectSlowFallVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	AHoldingObject* HoldingObject = Cast<AHoldingObject>(OtherActor);
	if (HoldingObject)
	{
		// HoldingObject에 SlowFallVolume 안에 있다고 알림
		HoldingObject->SetInSlowFallVolume(true);
		UE_LOG(LogTemp, Warning, TEXT("[ObjectSlowFallVolume] %s entered SlowFallVolume"), *HoldingObject->GetName());
	}
}

void AObjectSlowFallVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	AHoldingObject* HoldingObject = Cast<AHoldingObject>(OtherActor);
	if (HoldingObject)
	{
		// HoldingObject에 SlowFallVolume 밖으로 나갔다고 알림
		HoldingObject->SetInSlowFallVolume(false);
		UE_LOG(LogTemp, Warning, TEXT("[ObjectSlowFallVolume] %s leaved SlowFallVolume"), *HoldingObject->GetName());
	}
}

