// Rail Agliullin Dev. All Rights Reserved


#include "GameFrameworks/MD_GameState.h"

#include "Characters/MD_CharacterBase.h"
#include "GameModes/MD_GameMode.h"
#include "Net/UnrealNetwork.h"

void AMD_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMD_GameState, PickedHeroClasses);
}


bool AMD_GameState::IsHeroAlreadyPicked(TSubclassOf<AMD_CharacterBase> InHeroClass) const
{
	return PickedHeroClasses.Contains(InHeroClass);
}

bool AMD_GameState::AreAllHeroesSelected() const
{
	const int32 PlayerCount = PlayerArray.Num();
	const int32 PickedCount = PickedHeroClasses.Num();
	
	return PlayerCount > 0 && PickedCount >= PlayerCount;
}

void AMD_GameState::MarkHeroAsPicked(TSubclassOf<AMD_CharacterBase> InHeroClass)
{
	if (HasAuthority() && InHeroClass)
	{
		PickedHeroClasses.AddUnique(InHeroClass);
		FString RoleString = HasAuthority() ? TEXT("ListenServer-Host") : TEXT("Remote-Client");
		UE_LOG(LogTemp, Warning, TEXT("[%s] Герой %s теперь занят!"), *RoleString, *InHeroClass->GetName());
		
		if (AreAllHeroesSelected())
		{
			if (AMD_GameMode* GM = Cast<AMD_GameMode>(GetWorld()->GetAuthGameMode()))
			{
				GM->SetMatchStage(InProgress);
			}
		}
	}
}

