// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/CatBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACatBase::ACatBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create SpringArm component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = SpringArmLength;
	SpringArmComponent->bUsePawnControlRotation = bUsePawnControlRotation;
	SpringArmComponent->bInheritPitch = bInheritPitch;
	SpringArmComponent->bInheritYaw = bInheritYaw;
	SpringArmComponent->bInheritRoll = bInheritRoll;
	SpringArmComponent->SetRelativeLocation(SpringArmOffset);
	SpringArmComponent->SetRelativeRotation(FRotator(SpringArmPitch, 0.0f, 0.0f));

	// Create Camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	// Configure character movement
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void ACatBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply Blueprint settings when the game starts
	ApplyBlueprintSettings();
}

// Called every frame
void ACatBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACatBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACatBase::ApplyBlueprintSettings()
{
	if (SpringArmComponent)
	{
		SpringArmComponent->TargetArmLength = SpringArmLength;
		SpringArmComponent->bUsePawnControlRotation = bUsePawnControlRotation;
		SpringArmComponent->bInheritPitch = bInheritPitch;
		SpringArmComponent->bInheritYaw = bInheritYaw;
		SpringArmComponent->bInheritRoll = bInheritRoll;
		SpringArmComponent->SetRelativeLocation(SpringArmOffset);
		SpringArmComponent->SetRelativeRotation(FRotator(SpringArmPitch, 0.0f, 0.0f));
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
		GetCharacterMovement()->JumpZVelocity = JumpVelocity;
		GetCharacterMovement()->AirControl = AirControl;
	}
}

