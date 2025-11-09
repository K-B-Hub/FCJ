// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/DebuffVolume.h"

#include "PlayerCharacter/CatBase.h"
#include "Components/BoxComponent.h"

// Sets default values
ADebuffVolume::ADebuffVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
    
	SlowBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SlowBox"));
	RootComponent = SlowBox;
	SlowBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SlowBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	SlowBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void ADebuffVolume::BeginPlay()
{
	Super::BeginPlay();
	SlowBox->OnComponentBeginOverlap.AddDynamic(this, &ADebuffVolume::OnOverlapBegin);
	SlowBox->OnComponentEndOverlap.AddDynamic(this, &ADebuffVolume::OnOverlapEnd);

}

void ADebuffVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat)
	{
		// Character 내부의 Server RPC를 호출 (클라이언트에서도 자동으로 서버로 전달됨)
		Cat->ApplySpeedModifier(SpeedMultiplier);
	}
}

void ADebuffVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat)
	{
		// 속도 복구 (1.0f는 원래 속도)
		Cat->ApplySpeedModifier(1.0f);
	}
}
