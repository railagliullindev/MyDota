// Rail Agliullin Dev. All Rights Reserved

#include "GameModes/MD_GameMode.h"

#include "Characters/MD_CharacterBase.h"
#include "Controllers/MD_PlayerController.h"
#include "GameFrameworks/MD_GameState.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Systems/FogOfWar/FogOfWarManager.h"

AMD_GameMode::AMD_GameMode()
{
	PlayerControllerClass = AMD_PlayerController::StaticClass();
	PlayerStateClass = AMD_PlayerState::StaticClass();
	GameStateClass = AMD_GameState::StaticClass();

	MatchStage = EMatchStage::None;
}

void AMD_GameMode::BeginPlay()
{
	Super::BeginPlay();

	GS = GetGameState<AMD_GameState>();
	checkf(GS, TEXT("Game state is not AMD_GameState"));

	// Спавним менеджер для Radiant
	RadiantFogManager = GetWorld()->SpawnActor<AFogOfWarManager>(FogManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (RadiantFogManager) RadiantFogManager->AssignedTeamID = EMDTeam::Radiant;

	// Спавним менеджер для Dire
	DireFogManager = GetWorld()->SpawnActor<AFogOfWarManager>(FogManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (DireFogManager) DireFogManager->AssignedTeamID = EMDTeam::Dire;

	MoveToNextStage();
}

void AMD_GameMode::PostLogin(APlayerController* NewPlayer)
{
	if (MatchStage != EMatchStage::WaitingForPlayers)
	{
		// Игрок пытается подключиться не на той стадии - кикаем
		NewPlayer->ConsoleCommand("disconnect");
		return;
	}

	if (!NewPlayer) return;

	Super::PostLogin(NewPlayer);

	InitializePlayerData(NewPlayer);

	SpawnCameraForPlayer(NewPlayer);
}

void AMD_GameMode::Logout(AController* Exiting)
{
	UE_LOG(LogTemp, Warning, TEXT("@@@ LOGOUT %s"), *Exiting->GetName());

	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			if (GS)
			{
				// Удаляем игрока через GameState (вызовет делегаты!)
				GS->RemovePlayer(PS);
			}
		}
	}

	Super::Logout(Exiting);
}

void AMD_GameMode::ProcessHeroSelection(const APlayerController* PC, const int32 RequestedHeroId)
{
	if (!GS || !PC) return;
	if (GS->IsHeroAlreadyPicked(RequestedHeroId)) return;

	if (AMD_PlayerState* PS = PC->GetPlayerState<AMD_PlayerState>())
	{
		PS->HeroId = RequestedHeroId;
		PS->OnRep_HeroId();

		// Обновляем GameState (вызовет делегат OnPlayerHeroSelected!)
		GS->UpdatePlayerHero(PS, RequestedHeroId);

		if (GS->AreAllHeroesSelected())
		{
			MoveToNextStage();
		}
	}
}

FVector AMD_GameMode::GetBaseLocation(const EMDTeam Team) const
{
	if (Team == EMDTeam::Radiant) return RadiantSpawnLocation;
	if (Team == EMDTeam::Dire) return DireSpawnLocation;

	return FVector::ZeroVector; // Дефолт
}

void AMD_GameMode::InitializePlayerData(const APlayerController* NewPC) const
{
	AMD_PlayerState* PS = NewPC->GetPlayerState<AMD_PlayerState>();
	if (!PS || !GS) return;

	EMDTeam SetTeam = GS->GetSmallestTeam();
	PS->Team = SetTeam;

	// Обновляем GameState
	GS->UpdatePlayerTeam(PS, SetTeam);
}

void AMD_GameMode::WaitingForPlayers()
{
	StartStageTimer(WaitingForPlayersTime);
}

void AMD_GameMode::Draft()
{
	StartStageTimer(DraftTime);
}

void AMD_GameMode::PrepareForBattle()
{
	StartStageTimer(PrepareForBattleTime);
}

void AMD_GameMode::PreGame()
{
	// StartStageTimer(PreGameTime);

	StartGameClock();

	if (RadiantFogManager && DireFogManager)
	{
		RadiantFogManager->StartFogOfWar();
		DireFogManager->StartFogOfWar();
	}
	else
	{
		UE_LOG(LogGameMode, Error, TEXT("Fog managers not created"));
	}

	// Проходим по всем контроллерам в матче
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMD_PlayerController* PC = Cast<AMD_PlayerController>(It->Get());
		if (!PC) continue;

		AMD_PlayerState* PS = PC->GetPlayerState<AMD_PlayerState>();

		// Проверяем, выбрал ли игрок героя
		if (PS && PS->SelectedHeroClass)
		{
			PC->SetMatchMode(MatchStage);

			// Находим точку спавна (например, PlayerStart)
			// AActor* SpawnPoint = GetBaseLocation(FindPlayerStart(PC);
			FVector Loc = GetBaseLocation(PC->GetTeam()); // SpawnPoint ? SpawnPoint->GetActorLocation() : FVector::ZeroVector;

			FActorSpawnParameters HeroSpawnParams;
			HeroSpawnParams.Owner = PC;
			HeroSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 3. Спавним именно тот класс, который лежит в PlayerState
			AMD_CharacterBase* NewHero = GetWorld()->SpawnActor<AMD_CharacterBase>(PS->SelectedHeroClass, Loc, FRotator::ZeroRotator, HeroSpawnParams);

			if (NewHero)
			{
				NewHero->SetOwner(PC);
				NewHero->SetPlayerState(PS);
				PC->SetHero(NewHero);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Игрок %s не выбрал героя к началу матча!"), *PC->GetName());
		}
	}
}

void AMD_GameMode::InProgress()
{
}

void AMD_GameMode::PostGame()
{
}

void AMD_GameMode::SpawnCameraForPlayer(APlayerController* NewPlayer)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (AActor* PlayerStart = FindPlayerStart(NewPlayer))
	{
		SpawnLocation = PlayerStart->GetActorLocation();
		SpawnRotation = PlayerStart->GetActorRotation();
	}

	APawn* CameraPawn = GetWorld()->SpawnActor<APawn>(CameraPawnClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (CameraPawn)
	{
		NewPlayer->Possess(CameraPawn);
	}
}

void AMD_GameMode::StartStageTimer(const float Duration)
{
	GetWorldTimerManager().ClearTimer(StageTimerHandle);

	MatchTime = Duration;

	// Конечная точка = текущее серверное время + длительность
	GS->StageEndTime = GS->GetServerWorldTimeSeconds() + Duration;

	// Запускаем ежесекундный тик для обновления оставшегося времени
	GetWorldTimerManager().SetTimer(StageTimerHandle, this, &AMD_GameMode::UpdateStageTimer, 1.0f, true);
}

void AMD_GameMode::UpdateStageTimer()
{
	MatchTime -= 1.0f;

	if (MatchTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(StageTimerHandle);
		MoveToNextStage();
	}
}

void AMD_GameMode::StartGameClock()
{
	MatchTime = GS->GetServerWorldTimeSeconds() + PreGameTime;
	GS->MatchStartTime = MatchTime;

	GetWorldTimerManager().ClearTimer(StageTimerHandle);
}

void AMD_GameMode::MoveToNextStage()
{
	uint8 NextIndex = static_cast<uint8>(MatchStage) + 1;

	// Проверка на выход за пределы Enum
	if (NextIndex >= static_cast<uint8>(EMatchStage::PostGame) + 1) return;

	EMatchStage NewStage = static_cast<EMatchStage>(NextIndex);
	MatchStage = NewStage;

	UE_LOG(LogTemp, Warning, TEXT("Update MatchStage - %hhd"), MatchStage)

	// Реплицируем стейт через GameState
	GS->MatchStage = NewStage;

	// Настраиваем логику новой стадии
	switch (MatchStage)
	{
		case EMatchStage::WaitingForPlayers: WaitingForPlayers(); break;
		case EMatchStage::Draft: Draft(); break;
		case EMatchStage::PrepareForBattle: PrepareForBattle(); break;
		case EMatchStage::PreGame: PreGame(); break;
		case EMatchStage::InProgress: InProgress(); break;
		case EMatchStage::PostGame: PostGame(); break;
		default: break;
	}

	GS->SetMatchStage(NewStage);
}
