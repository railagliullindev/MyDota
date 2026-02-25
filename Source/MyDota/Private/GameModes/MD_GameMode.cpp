// Rail Agliullin Dev. All Rights Reserved


#include "GameModes/MD_GameMode.h"

#include "Characters/MD_CharacterBase.h"
#include "Controllers/MD_PlayerController.h"
#include "Pawns/MD_CameraPawn.h"

void AMD_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!NewPlayer) return;
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = GetInstigator();
	
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
	
	AMD_CharacterBase* Hero = GetWorld()->SpawnActor<AMD_CharacterBase>(HeroClass, SpawnLocation, SpawnRotation, SpawnParameters);
	if (Hero)
	{
		if (AMD_PlayerController* DotaPC = Cast<AMD_PlayerController>(NewPlayer))
		{
			DotaPC->SetHero(Hero);
		}
	}
}
