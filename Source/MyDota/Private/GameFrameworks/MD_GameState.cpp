// Rail Agliullin Dev. All Rights Reserved

#include "GameFrameworks/MD_GameState.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "MD_GameplayTags.h"
#include "MyDotaStructTypes.h"
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

bool AMD_GameState::AreAllHeroesSelected() const
{
	if (HasAuthority())
	{
		return HeroesInfo.FindByPredicate(
				   [](const FMatchHeroesInfo& P)
				   {
					   return P.HeroId == -1 || P.PlayerId == -1;
				   }) == nullptr;
	}
	return false;
}

bool AMD_GameState::IsHeroAlreadyPicked(const int32 HeroIndex) const
{
	if (HasAuthority())
	{
		return HeroesInfo.FindByPredicate(
				   [HeroIndex](const FMatchHeroesInfo& P)
				   {
					   return P.HeroId == HeroIndex;
				   }) != nullptr;
	}
	return false;
}

void AMD_GameState::RegisterHeroSelection(const int32 InPlayerId, const int32 HeroId)
{
	if (!HasAuthority()) return;

	FMatchHeroesInfo* Info = HeroesInfo.FindByPredicate(
		[InPlayerId](const FMatchHeroesInfo& P)
		{
			return P.PlayerId == InPlayerId;
		});

	if (Info)
	{
		Info->HeroId = HeroId;
		UE_LOG(LogTemp, Log, TEXT("AMD_GameState::RegisterHeroSelection ok [%d]"), HeroId);
	}
	MARK_PROPERTY_DIRTY_FROM_NAME(AMD_GameState, HeroesInfo, this);
}

void AMD_GameState::RegisterNewPlayer(const int32 PlayerId, const int32 TeamId)
{
	if (!HasAuthority()) return;

	FMatchHeroesInfo* Info = HeroesInfo.FindByPredicate(
		[PlayerId](const FMatchHeroesInfo& P)
		{
			return P.PlayerId == PlayerId;
		});

	if (Info)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player with %d id has registered"), PlayerId);
		return;
	}

	FMatchHeroesInfo RegisteredHeroesInfo;

	RegisteredHeroesInfo.PlayerId = PlayerId;
	RegisteredHeroesInfo.TeamId = TeamId;

	HeroesInfo.Add(RegisteredHeroesInfo);
	MARK_PROPERTY_DIRTY_FROM_NAME(AMD_GameState, HeroesInfo, this);
}
