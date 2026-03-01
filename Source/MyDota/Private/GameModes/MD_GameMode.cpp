// Rail Agliullin Dev. All Rights Reserved


#include "GameModes/MD_GameMode.h"

#include "Characters/MD_CharacterBase.h"
#include "Controllers/MD_PlayerController.h"
#include "GameFrameworks/MD_GameState.h"
#include "GameFrameworks/MD_PlayerState.h"

AMD_GameMode::AMD_GameMode()
{
	PlayerControllerClass = AMD_PlayerController::StaticClass();
	PlayerStateClass = AMD_PlayerState::StaticClass();
	GameStateClass = AMD_GameState::StaticClass();
	bIsTeamA = true;
}

void AMD_GameMode::BeginPlay()
{
	Super::BeginPlay();
	
	MD_GameState = GetGameState<AMD_GameState>();
	checkf(MD_GameState, TEXT("Game state is not AMD_GameState"));
	
	SetMatchStage(EMathStage::WaitingForPlayers);
}

void AMD_GameMode::PostLogin(APlayerController* NewPlayer)
{
	if (MatchStage != EMathStage::WaitingForPlayers) return;
	if (!NewPlayer) return;
	
	Super::PostLogin(NewPlayer);
	
	// Установка стороны
	AMD_PlayerState* PS = NewPlayer->GetPlayerState<AMD_PlayerState>();
	if (PS)
	{
		PS->bIsTeamA = bIsTeamA;
		bIsTeamA = !bIsTeamA;
	}
	
	SpawnCameraForPlayer(NewPlayer);
}

void AMD_GameMode::SetMatchStage(EMathStage NewStage)
{
	//if (MatchStage >= NewStage) return;
	
	MatchStage = NewStage;
	
	switch (MatchStage)
	{
	case EMathStage::WaitingForPlayers:
		UE_LOG(LogTemp, Warning, TEXT("AMD_GameMode::SetMatchStage WAITINGFORPLAYERS"));
		WaitingForPlayers();
		break;
	case EMathStage::Draft:
		UE_LOG(LogTemp, Warning, TEXT("AMD_GameMode::SetMatchStage DRAFT"));
		Draft();
		break;
	case EMathStage::PreGame:
		UE_LOG(LogTemp, Warning, TEXT("AMD_GameMode::SetMatchStage PRE GAME"));
		break;
	case EMathStage::InProgress:
		UE_LOG(LogTemp, Warning, TEXT("AMD_GameMode::SetMatchStage IN PROGRESS"));
		InProgress();
		break;
	case EMathStage::PostGame:
		UE_LOG(LogTemp, Warning, TEXT("AMD_GameMode::SetMatchStage POST GAME"));
		break;
	}
	
	MD_GameState->SetMatchStage(NewStage);
}

void AMD_GameMode::WaitingForPlayers()
{
	const float Delay = 2.0f; // Задержка в секундах

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
	{
		SetMatchStage(EMathStage::Draft);

	}, Delay, false);
}

void AMD_GameMode::Draft()
{
	
}

void AMD_GameMode::PreGame()
{
	
}

void AMD_GameMode::InProgress()
{
	// Проходим по всем контроллерам в матче
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMD_PlayerController* PC = Cast<AMD_PlayerController>(It->Get());
		if (!PC) continue;

		FString RoleString = HasAuthority() ? TEXT("ListenServer-Host") : TEXT("Remote-Client");
		UE_LOG(LogTemp, Log, TEXT("[%s] Перевод на старт - %s"),*RoleString, *PC->GetName());
		
		AMD_PlayerState* PS = PC->GetPlayerState<AMD_PlayerState>();
		
		// Проверяем, выбрал ли игрок героя
		if (PS && PS->SelectedHeroClass)
		{			
			PC->SetMatchMode(MatchStage);
			
			// Находим точку спавна (например, PlayerStart)
			AActor* SpawnPoint = FindPlayerStart(PC);
			FVector Loc = SpawnPoint ? SpawnPoint->GetActorLocation() : FVector::ZeroVector;

			FActorSpawnParameters HeroSpawnParams;
			HeroSpawnParams.Owner = PC;
			HeroSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 3. Спавним именно тот класс, который лежит в PlayerState
			AMD_CharacterBase* NewHero = GetWorld()->SpawnActor<AMD_CharacterBase>(
				PS->SelectedHeroClass, Loc, FRotator::ZeroRotator, HeroSpawnParams);

			if (NewHero)
			{
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
