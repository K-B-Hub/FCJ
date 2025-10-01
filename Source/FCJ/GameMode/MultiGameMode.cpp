// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MultiGameMode.h"
#include "PlayerController/MultiPlayerController.h"
#include "PlayerCharacter/CatBase.h"
#include "PlayerCharacter/AttackCat.h"
#include "PlayerCharacter/BiteCat.h"
#include "Subsystem/MultiSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerState.h"

AMultiGameMode::AMultiGameMode()
{
	// Set default classes (can be overridden in Blueprint)
	PlayerControllerClass = AMultiPlayerController::StaticClass();
	DefaultPawnClass = ACatBase::StaticClass();

	// 기본 캐릭터 클래스 설정
	AttackCatClass = AAttackCat::StaticClass();
	BiteCatClass = ABiteCat::StaticClass();
}

void AMultiGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Apply Blueprint-configured classes if they are set
	if (MultiPlayerControllerClass)
	{
		PlayerControllerClass = MultiPlayerControllerClass;
	}

	if (DefaultPawnClass_Multi)
	{
		DefaultPawnClass = DefaultPawnClass_Multi;
	}
}

void AMultiGameMode::HostLANGame()
{
	GetWorld()->ServerTravel("/Game/Levels/Test?listen");
}

void AMultiGameMode::JoinLANGame()
{
	APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (PC)
	{
		PC->ClientTravel("211.117.90.221", TRAVEL_Absolute);
	}
}

void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Apply saved display settings on game startup
	ApplySavedDisplaySettings();
}

void AMultiGameMode::ApplySavedDisplaySettings()
{
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (GameUserSettings)
	{
		GameUserSettings->LoadSettings();
		GameUserSettings->ApplySettings(false);
	}
}

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer) return;

	// Subsystem에서 저장된 역할 정보 가져오기
	int32 role = GetRoleFromSubsystem(NewPlayer);

	if (role >= 0 && role < 2)
	{
		PlayerRoles.Add(NewPlayer, role);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			FString::Printf(TEXT("Player assigned role from Subsystem: %d"), role));

		// 역할에 맞는 캐릭터 생성
		TSubclassOf<ACatBase> PawnClassToSpawn = nullptr;

		if (role == 0 && AttackCatClass)
		{
			// 1P = AttackCat
			PawnClassToSpawn = AttackCatClass;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Spawning AttackCat for Player 1"));
		}
		else if (role == 1 && BiteCatClass)
		{
			// 2P = BiteCat
			PawnClassToSpawn = BiteCatClass;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("Spawning BiteCat for Player 2"));
		}

		// 기존 폰 제거 및 새 캐릭터 생성
		if (PawnClassToSpawn)
		{
			if (NewPlayer->GetPawn())
			{
				NewPlayer->GetPawn()->Destroy();
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = NewPlayer;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AActor* StartSpot = FindPlayerStart(NewPlayer);
			FVector SpawnLocation = StartSpot ? StartSpot->GetActorLocation() : FVector::ZeroVector;
			FRotator SpawnRotation = StartSpot ? StartSpot->GetActorRotation() : FRotator::ZeroRotator;

			ACatBase* NewPawn = GetWorld()->SpawnActor<ACatBase>(PawnClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
			if (NewPawn)
			{
				NewPlayer->Possess(NewPawn);
			}
		}
	}
	else
	{
		// 역할이 없으면 기본 로직 사용
		int32 PlayerCount = PlayerRoles.Num();
		if (PlayerCount < 2)
		{
			PlayerRoles.Add(NewPlayer, PlayerCount);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
				FString::Printf(TEXT("Player assigned default role: %d"), PlayerCount));
		}
	}
}

int32 AMultiGameMode::GetRoleFromSubsystem(APlayerController* PC)
{
	if (!PC) return -1;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return -1;

	UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
	if (!SessionSubsystem) return -1;

	// PlayerState에서 UniqueNetId 가져오기
	if (PC->PlayerState)
	{
		FUniqueNetIdRepl UniqueId = PC->PlayerState->GetUniqueId();
		if (UniqueId.IsValid())
		{
			FString PlayerNetId = UniqueId->ToString();
			return SessionSubsystem->GetPlayerRole(PlayerNetId);
		}
	}

	return -1;
}

void AMultiGameMode::SwapPlayerRoles()
{
	if (PlayerRoles.Num() < 2) return;

	// 역할 교체
	TArray<APlayerController*> Controllers;
	PlayerRoles.GetKeys(Controllers);

	if (Controllers.Num() >= 2)
	{
		int32 Role1 = PlayerRoles[Controllers[0]];
		int32 Role2 = PlayerRoles[Controllers[1]];

		PlayerRoles[Controllers[0]] = Role2;
		PlayerRoles[Controllers[1]] = Role1;

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Player roles swapped"));
	}
}

int32 AMultiGameMode::GetPlayerRole(APlayerController* PC) const
{
	const int32* role = PlayerRoles.Find(PC);
	return role ? *role : -1;
}

