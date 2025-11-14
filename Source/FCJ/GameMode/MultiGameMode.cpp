// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MultiGameMode.h"

#include "LobbyGameState.h"
#include "PlayerController/MultiPlayerController.h"
#include "PlayerCharacter/CatBase.h"
#include "PlayerCharacter/AttackCat.h"
#include "PlayerCharacter/BiteCat.h"
#include "Subsystem/MultiSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerState.h"
#include "Actor/Volumes/ZoneVolume.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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

	// 거리 체크 타이머 시작 (서버에서만 실행)
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			DistanceCheckTimerHandle,
			this,
			&AMultiGameMode::CheckCharacterDistance,
			DistanceCheckInterval,
			true  // 반복 실행
		);

		UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Distance check timer started - Interval: %.2f seconds, MaxDistance: %.2f"),
			DistanceCheckInterval, MaxAllowedDistance);
	}
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

	// PlayerState에서 UniqueNetId 가져오기
	if (!PC->PlayerState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerState is null"));
		return -1;
	}

	FUniqueNetIdRepl UniqueId = PC->PlayerState->GetUniqueId();
	if (!UniqueId.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UniqueNetId is invalid"));
		return -1;
	}

	FString PlayerNetId = UniqueId->ToString();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
		FString::Printf(TEXT("Getting role for NetId: %s"), *PlayerNetId));

	// MultiSessionSubsystem에서 역할 가져오기 (레벨 전환 시에도 유지됨)
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
		if (SessionSubsystem)
		{
			int32 role = SessionSubsystem->GetPlayerRole(PlayerNetId);
			if (role >= 0)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
					FString::Printf(TEXT("Found role in Subsystem: %d"), role));
				return role;
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("No role found in Subsystem"));
			}
		}
	}

	// 폴백: LobbyGameState에서 가져오기 (로비에 있을 경우)
	ALobbyGameState* LobbyGS = GetWorld()->GetGameState<ALobbyGameState>();
	if (LobbyGS)
	{
		int32 role = LobbyGS->GetPlayerRole(PlayerNetId);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
			FString::Printf(TEXT("Found role in LobbyGameState (fallback): %d"), role));
		return role;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No role found anywhere"));
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

void AMultiGameMode::CheckCharacterDistance()
{
	// 모든 플레이어 캐릭터 수집
	TArray<ACatBase*> PlayerCharacters;
	for (auto& Pair : PlayerRoles)
	{
		APlayerController* PC = Pair.Key;
		if (PC && PC->GetPawn())
		{
			ACatBase* CatCharacter = Cast<ACatBase>(PC->GetPawn());
			if (CatCharacter)
			{
				PlayerCharacters.Add(CatCharacter);
			}
		}
	}

	// 플레이어가 2명 미만이면 체크하지 않음
	if (PlayerCharacters.Num() < 2)
	{
		return;
	}

	// 두 캐릭터 간의 거리 계산
	ACatBase* Character1 = PlayerCharacters[0];
	ACatBase* Character2 = PlayerCharacters[1];

	float Distance = FVector::Dist(Character1->GetActorLocation(), Character2->GetActorLocation());

	// 최대 허용 거리를 초과하면
	if (Distance > MaxAllowedDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Characters too far apart! Distance: %.2f (Max: %.2f)"),
			Distance, MaxAllowedDistance);

		// 각 캐릭터가 속한 ZoneVolume 확인
		AZoneVolume* Zone1 = GetCharacterZone(Character1);
		AZoneVolume* Zone2 = GetCharacterZone(Character2);

		// 두 캐릭터 모두 ZoneVolume에 속해있어야 함
		if (!Zone1 || !Zone2)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] One or both characters not in ZoneVolume - Zone1: %s, Zone2: %s"),
				Zone1 ? *Zone1->GetName() : TEXT("None"), Zone2 ? *Zone2->GetName() : TEXT("None"));
			return;
		}

		int32 ZoneNumber1 = Zone1->GetZoneNumber();
		int32 ZoneNumber2 = Zone2->GetZoneNumber();

		// 같은 구역이면 아무것도 하지 않음
		if (ZoneNumber1 == ZoneNumber2)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Both characters in same zone (%d) - No teleport needed"),
				ZoneNumber1);
			return;
		}

		// 더 낮은 번호의 구역에 있는 캐릭터 쪽으로 순간이동
		if (ZoneNumber1 < ZoneNumber2)
		{
			// Character1이 더 낮은 구역 -> Character2를 Character1 쪽으로 이동
			UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Teleporting Character2 (Zone %d) to Character1 (Zone %d)"),
				ZoneNumber2, ZoneNumber1);
			TeleportCharacter(Character2, Character1);
		}
		else
		{
			// Character2가 더 낮은 구역 -> Character1을 Character2 쪽으로 이동
			UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Teleporting Character1 (Zone %d) to Character2 (Zone %d)"),
				ZoneNumber1, ZoneNumber2);
			TeleportCharacter(Character1, Character2);
		}
	}
}

AZoneVolume* AMultiGameMode::GetCharacterZone(ACatBase* Character)
{
	if (!Character)
	{
		return nullptr;
	}

	// 월드의 모든 ZoneVolume 가져오기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZoneVolume::StaticClass(), FoundActors);

	// 캐릭터가 속한 ZoneVolume 찾기
	for (AActor* Actor : FoundActors)
	{
		AZoneVolume* Zone = Cast<AZoneVolume>(Actor);
		if (Zone && Zone->GetVolumeBox())
		{
			// 캐릭터가 이 ZoneVolume과 오버랩되는지 확인
			if (Zone->GetVolumeBox()->IsOverlappingActor(Character))
			{
				UE_LOG(LogTemp, Log, TEXT("[MultiGameMode] Character %s is in Zone %s (Number: %d)"),
					*Character->GetName(), *Zone->GetName(), Zone->GetZoneNumber());
				return Zone;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Character %s is not in any ZoneVolume"),
		*Character->GetName());
	return nullptr;
}

void AMultiGameMode::TeleportCharacter(ACatBase* CharacterToTeleport, ACatBase* TargetCharacter)
{
	if (!CharacterToTeleport || !TargetCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiGameMode] TeleportCharacter - Invalid characters"));
		return;
	}

	// 목표 캐릭터 앞쪽으로 순간이동 (약간 떨어진 위치)
	FVector TargetLocation = TargetCharacter->GetActorLocation();
	FVector TargetForward = TargetCharacter->GetActorForwardVector();
	FVector TeleportLocation = TargetLocation + (TargetForward * 200.0f);  // 200유닛 앞

	// 순간이동 실행
	bool bSuccess = CharacterToTeleport->TeleportTo(TeleportLocation, CharacterToTeleport->GetActorRotation());

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MultiGameMode] Successfully teleported %s to %s's location"),
			*CharacterToTeleport->GetName(), *TargetCharacter->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MultiGameMode] Failed to teleport %s"),
			*CharacterToTeleport->GetName());
	}
}

