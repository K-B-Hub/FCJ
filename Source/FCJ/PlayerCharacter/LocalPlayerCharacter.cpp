// Copyright Epic Games, Inc. All Rights Reserved.


#include "PlayerCharacter/LocalPlayerCharacter.h"
#include "Camera/CameraComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter/FrontCat.h"
#include "PlayerCharacter/BackCat.h"

#include "EnhancedInputComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ALocalPlayerCharacter::ALocalPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ALocalPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (BackCatClass) // Blueprint에서 설정한 클래스
	{
		FVector SpawnLocation = GetActorLocation() + FVector(-100.f, 0.f, 0.f);
		BackCat = GetWorld()->SpawnActor<ABackCat>(BackCatClass, SpawnLocation, GetActorRotation());
		BackCat->bUseControllerRotationYaw = false;
		BackCat->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
    
	if (FrontCatClass)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(100.f, 0.f, 0.f);
		FrontCat = GetWorld()->SpawnActor<AFrontCat>(FrontCatClass, SpawnLocation, GetActorRotation());
		FrontCat->bUseControllerRotationYaw = false;
		FrontCat->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	if (BackCat)
	{
		// BackCat용 AIController 생성 및 할당
		AAIController* BackCatController = GetWorld()->SpawnActor<AAIController>();
		BackCatController->Possess(BackCat);
        
		UE_LOG(LogTemp, Warning, TEXT("BackCat Controller assigned: %s"), 
			   *BackCatController->GetName());
	}
    
	if (FrontCat)
	{
		// FrontCat용 AIController 생성 및 할당
		AAIController* FrontCatController = GetWorld()->SpawnActor<AAIController>();
		FrontCatController->Possess(FrontCat);
	}
}

// Called every frame
void ALocalPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AFrontCat* ALocalPlayerCharacter::GetFrontCat() const
{
	return FrontCat;
}

ABackCat* ALocalPlayerCharacter::GetBackCat() const
{
	return BackCat;
}

void ALocalPlayerCharacter::HandleBackCatMovement(const FInputActionValue& Value)
{
	if (BackCat)
	{
		const FVector2D Input2D = Value.Get<FVector2D>();
		UE_LOG(LogTemp, Warning, TEXT("Moving BackCat - Input: X=%f, Y=%f"), Input2D.X, Input2D.Y);

		if (UCharacterMovementComponent* MovementComp = BackCat->GetCharacterMovement())
		{
			// 1. 이동 벡터 계산
			FVector MoveVector = FVector(Input2D.Y, Input2D.X, 0.f);

			// 2. 이동
			BackCat->AddMovementInput(MoveVector.GetSafeNormal(), MoveVector.Size());

			// 3. 이동 방향으로 회전
			if (!MoveVector.IsNearlyZero())
			{
				FRotator TargetRotation = MoveVector.Rotation();
				FRotator NewRotation = FMath::RInterpTo(
					BackCat->GetActorRotation(),
					TargetRotation,
					GetWorld()->GetDeltaSeconds(),
					10.0f // 회전 속도 조절 가능
				);

				BackCat->SetActorRotation(NewRotation);
			}

			// 4. 디버그 출력
			UE_LOG(LogTemp, Warning, TEXT("BackCat Location: %s"),
				*BackCat->GetActorLocation().ToString());
		}
	}
}

void ALocalPlayerCharacter::HandleFrontCatMovement(const FInputActionValue& Value)
{
	if (FrontCat)
	{
		const FVector2D Input2D = Value.Get<FVector2D>();
		UE_LOG(LogTemp, Warning, TEXT("Moving FrontCat - Input: X=%f, Y=%f"), Input2D.X, Input2D.Y);

		if (UCharacterMovementComponent* MovementComp = FrontCat->GetCharacterMovement())
		{
			// 1. 이동 벡터 계산
			FVector MoveVector = FVector(Input2D.Y, Input2D.X, 0.f);

			// 2. 이동
			FrontCat->AddMovementInput(MoveVector.GetSafeNormal(), MoveVector.Size());

			// 3. 이동 방향으로 회전
			if (!MoveVector.IsNearlyZero())
			{
				FRotator TargetRotation = MoveVector.Rotation();
				FRotator NewRotation = FMath::RInterpTo(
					FrontCat->GetActorRotation(),
					TargetRotation,
					GetWorld()->GetDeltaSeconds(),
					10.0f // 회전 속도 조절 가능
				);

				FrontCat->SetActorRotation(NewRotation);
			}

			// 4. 디버그 출력
			UE_LOG(LogTemp, Warning, TEXT("FrontCat Location: %s"),
				*FrontCat->GetActorLocation().ToString());
		}
	}
}