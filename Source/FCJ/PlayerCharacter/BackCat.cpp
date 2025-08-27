// Fill out your copyright notice in the Description page of Project Settings.


#include "BackCat.h"

// Sets default values
ABackCat::ABackCat()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABackCat::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABackCat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABackCat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

