// Rail Agliullin Dev. All Rights Reserved

#include "Actors/MD_ActorsBase.h"

#include "Systems/FogOfWar/FogOfWarManager.h"

// Sets default values
AMD_ActorsBase::AMD_ActorsBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = false;
}

EMDTeam AMD_ActorsBase::GetTeam() const
{
	return Team;
}

void AMD_ActorsBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AFogOfWarManager* FogManager = AFogOfWarManager::Get(this, (uint8)GetTeam());
		if (FogManager)
		{
			// Регистрируем юнит как источник обзора
			FogManager->RegisterSource(this, 300.0f);
			UE_LOG(LogTemp, Warning, TEXT("Server: Registered %s as Vision Source"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Server: FogManager NOT FOUND during BeginPlay!"));
		}
	}
}
