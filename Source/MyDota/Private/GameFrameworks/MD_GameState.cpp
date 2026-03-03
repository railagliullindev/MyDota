// Rail Agliullin Dev. All Rights Reserved

#include "GameFrameworks/MD_GameState.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "Characters/MD_CharacterBase.h"
#include "Controllers/MD_PlayerController.h"
#include "GameModes/MD_GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Pawns/MD_CameraPawn.h"

void AMD_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMD_GameState, PickedHeroClasses);
	DOREPLIFETIME(AMD_GameState, MathStage);
}

void AMD_GameState::OnRep_MatchStage()
{
	switch (MathStage)
	{
		case EMathStage::Draft:
			UE_LOG(LogTemp, Warning, TEXT("Клиент: Начался драфт"));

			// Находим локальный контроллер и говорим ему активировать Draft-абилку
			if (AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				if (AMD_CameraPawn* CamPawn = Cast<AMD_CameraPawn>(PC->GetPawn()))
				{
					CamPawn->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer{MyDotaTags::Ability_ShowDraft});
				}
			}
			break;

		case EMathStage::InProgress:
			UE_LOG(LogTemp, Warning, TEXT("Клиент: Игра началась"));

			if (AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				if (AMD_CameraPawn* CamPawn = Cast<AMD_CameraPawn>(PC->GetPawn()))
				{
					CamPawn->GetMDAbilitySystemComponent()->CancelAbilityWithTag(MyDotaTags::Ability_ShowDraft);
					CamPawn->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer{MyDotaTags::Ability_ShowGameplayHUD});
				}
			}
			break;

		default: break;
	}
}

void AMD_GameState::SetMatchStage(EMathStage NewStage)
{
	if (HasAuthority())
	{
		MathStage = NewStage;

		OnRep_MatchStage();
	}
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
				GM->SetMatchStage(EMathStage::InProgress);
			}
		}
	}
}
