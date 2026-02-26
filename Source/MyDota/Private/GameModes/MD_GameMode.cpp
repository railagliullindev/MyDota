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

void AMD_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!NewPlayer) return;
	
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
	
	AMD_PlayerState* PS = NewPlayer->GetPlayerState<AMD_PlayerState>();
	if (PS)
	{
		PS->bIsTeamA = bIsTeamA;
		bIsTeamA = !bIsTeamA;
	}
}

void AMD_GameMode::StartMatch()
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
			UE_LOG(LogTemp, Log, TEXT("[%s]%s PS Valid, HideDraftWidget"),*RoleString, *PC->GetName());
			
			PC->HideDraftWidget();
			
			// Находим точку спавна (например, PlayerStart)
			AActor* SpawnPoint = FindPlayerStart(PC);
			FVector Loc = SpawnPoint ? SpawnPoint->GetActorLocation() : FVector::ZeroVector;

			FActorSpawnParameters HeroSpawnParams;
			HeroSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 3. Спавним именно тот класс, который лежит в PlayerState
			AMD_CharacterBase* NewHero = GetWorld()->SpawnActor<AMD_CharacterBase>(
				PS->SelectedHeroClass, Loc, FRotator::ZeroRotator, HeroSpawnParams);

			if (NewHero)
			{
				// 4. Привязываем героя к контроллеру (наша старая функция)
				PC->SetHero(NewHero);
				
				// Опционально: можно переместить камеру к герою или удалить DraftCamera
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Игрок %s не выбрал героя к началу матча!"), *PC->GetName());
		}
	}
	
	Super::StartMatch();
}
