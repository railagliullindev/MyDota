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
}

void AMD_GameMode::BeginPlay()
{
	Super::BeginPlay();

	GS = GetGameState<AMD_GameState>();
	checkf(GS, TEXT("Game state is not AMD_GameState"));

	SetMatchStage(EMatchStage::WaitingForPlayers);

	// Спавним менеджер для Radiant
	RadiantFogManager = GetWorld()->SpawnActor<AFogOfWarManager>(FogManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (RadiantFogManager) RadiantFogManager->AssignedTeamID = EMDTeam::Radiant;

	// Спавним менеджер для Dire
	DireFogManager = GetWorld()->SpawnActor<AFogOfWarManager>(FogManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	if (DireFogManager) DireFogManager->AssignedTeamID = EMDTeam::Dire;
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
			SetMatchStage(EMatchStage::PreGame);
		}
	}
}

void AMD_GameMode::SetMatchStage(EMatchStage NewStage)
{
	MatchStage = NewStage;

	switch (MatchStage)
	{
		case EMatchStage::WaitingForPlayers: WaitingForPlayers(); break;
		case EMatchStage::Draft: Draft(); break;
		case EMatchStage::PreGame: PreGame(); break;
		case EMatchStage::InProgress: InProgress(); break;
		case EMatchStage::PostGame: UE_LOG(LogTemp, Log, TEXT("AMD_GameMode::SetMatchStage POST GAME")); break;
	}

	GS->SetMatchStage(NewStage);
}

FVector AMD_GameMode::GetBaseLocation(EMDTeam Team) const
{
	if (Team == EMDTeam::Radiant) return RadiantSpawnLocation;
	if (Team == EMDTeam::Dire) return DireSpawnLocation;

	return FVector::ZeroVector; // Дефолт
}

void AMD_GameMode::InitializePlayerData(APlayerController* NewPC)
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
	const float Delay = 2.0f; // Задержка в секундах

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			SetMatchStage(EMatchStage::Draft);
		},
		Delay, false);
}

void AMD_GameMode::Draft()
{
}

void AMD_GameMode::PreGame()
{
	// Задержка 3 секунды перед добавлением данных
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;

	// Передаем контроллер в лямбду, чтобы знать, кого добавлять
	TimerDel.BindLambda(
		[this]()
		{
			SetMatchStage(EMatchStage::InProgress);
		});

	GetWorldTimerManager().SetTimer(TimerHandle, TimerDel, 3.0f, false);
}

void AMD_GameMode::InProgress()
{
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
