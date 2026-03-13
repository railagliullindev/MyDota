// Rail Agliullin Dev. All Rights Reserved

#include "GameFrameworks/MD_GameState.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "MD_GameplayTags.h"
#include "MyDotaStructTypes.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "Controllers/MD_PlayerController.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "GameModes/MD_GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Pawns/MD_CameraPawn.h"

AMD_GameState::AMD_GameState()
{
	bReplicates = true;

	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
}

void AMD_GameState::BeginPlay()
{
	Super::BeginPlay();
}

void AMD_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AMD_GameState, PlayersInfo, COND_SkipOwner, REPNOTIFY_Always);
	// DOREPLIFETIME_CONDITION(AMD_GameState, PlayersInfo, COND_None);
	// DOREPLIFETIME(AMD_GameState, PlayersInfo);

	DOREPLIFETIME(AMD_GameState, MatchStage);
	DOREPLIFETIME_CONDITION(AMD_GameState, LastTeamChange, COND_SkipOwner);

	DOREPLIFETIME(AMD_GameState, StageEndTime);
	DOREPLIFETIME(AMD_GameState, MatchStartTime);
}

int32 AMD_GameState::GetPlayersCountInTeam(const EMDTeam Team) const
{
	int32 Count = 0;
	for (const FPlayerTeamInfo& Info : PlayersInfo)
	{
		if (Info.GetTeam() == Team)
		{
			Count++;
		}
	}
	return Count;
}

bool AMD_GameState::IsTeamFull(const EMDTeam Team) const
{
	return GetPlayersCountInTeam(Team) >= MaxPlayersPerTeam;
}

bool AMD_GameState::CanJoinTeam(const EMDTeam Team) const
{
	if (Team == EMDTeam::None) return false;
	return !IsTeamFull(Team);
}

EMDTeam AMD_GameState::GetSmallestTeam() const
{
	const int32 RadiantCount = GetPlayersCountInTeam(EMDTeam::Radiant);
	const int32 DireCount = GetPlayersCountInTeam(EMDTeam::Dire);

	return (RadiantCount <= DireCount) ? EMDTeam::Radiant : EMDTeam::Dire;
}

void AMD_GameState::UpdatePlayerTeam(APlayerState* PlayerState, const EMDTeam NewTeam)
{
	FString NetPrefix = GetWorld()->GetNetMode() == NM_Client ? FString::Printf(TEXT("Client %d"), UE::GetPlayInEditorID()) : TEXT("Server");

	// Создаем новую запись
	FPlayerTeamInfo NewInfo;
	NewInfo.PlayerId = PlayerState->GetPlayerId();
	NewInfo.SetTeam(NewTeam);
	NewInfo.PlayerName = PlayerState->GetPlayerName();

	UE_LOG(LogTemp, Warning, TEXT("@@@ [ %s ] Set PI. PlayerID - %d, Team - %d"), *NetPrefix, NewInfo.PlayerId, NewInfo.GetTeam());

	PlayersInfo.Add(NewInfo);
}

void AMD_GameState::UpdatePlayerHero(APlayerState* PlayerState, const int32 NewHeroId)
{
	if (!HasAuthority() || !PlayerState) return;

	UE_LOG(LogTemp, Log, TEXT("AMD_GameState::UpdatePlayerHero"));

	int32 PlayerId = PlayerState->GetPlayerId();
	EMDTeam PlayerTeam = EMDTeam::None;

	FPlayerTeamInfo* Info = PlayersInfo.FindByPredicate(
		[PlayerId](const FPlayerTeamInfo& PlayerInfo)
		{
			return PlayerInfo.PlayerId == PlayerId;
		});

	if (Info)
	{
		Info->HeroId = NewHeroId;
		PlayerTeam = Info->GetTeam();

		MARK_PROPERTY_DIRTY_FROM_NAME(AMD_GameState, PlayersInfo, this);

		// ЗАПИСЫВАЕМ ИЗМЕНЕНИЕ ДЛЯ РЕПЛИКАЦИИ
		UE_LOG(LogTemp, Log, TEXT("AMD_GameState::UpdatePlayerHero. ChangeType = %d"), (uint8)EChangeType::HeroSelected);
		RecordChange(PlayerTeam, PlayerId, NewHeroId, EChangeType::HeroSelected);

		OnPlayerHeroSelected.Broadcast(PlayerId, NewHeroId, PlayerTeam);
		OnTeamCompositionChanged.Broadcast(PlayerTeam);
	}
}

void AMD_GameState::RemovePlayer(APlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState) return;

	int32 PlayerId = PlayerState->GetPlayerId();
	EMDTeam OldTeam = EMDTeam::None;

	// Находим команду игрока перед удалением
	const FPlayerTeamInfo* Info = PlayersInfo.FindByPredicate(
		[PlayerId](const FPlayerTeamInfo& PlayerInfo)
		{
			return PlayerInfo.PlayerId == PlayerId;
		});

	if (Info)
	{
		OldTeam = Info->GetTeam();
	}

	// Удаляем игрока
	int32 RemovedCount = PlayersInfo.RemoveAll(
		[PlayerId](const FPlayerTeamInfo& PlayerInfo)
		{
			return PlayerInfo.PlayerId == PlayerId;
		});

	if (RemovedCount > 0)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(AMD_GameState, PlayersInfo, this);

		// Записываем изменение для репликации
		RecordChange(OldTeam, PlayerId, -1, EChangeType::PlayerLeft);
		RecordChange(OldTeam, -1, -1, EChangeType::TeamChanged);

		// Оповещаем об уходе игрока
		OnPlayerLeftTeam.Broadcast(PlayerId, OldTeam);
		OnTeamCompositionChanged.Broadcast(OldTeam);

		UE_LOG(LogTemp, Log, TEXT("Player %d removed from GameState"), PlayerId);
	}
}

bool AMD_GameState::GetPlayerInfo(int32 PlayerId, FPlayerTeamInfo& OutInfo) const
{
	const FPlayerTeamInfo* Info = PlayersInfo.FindByPredicate(
		[PlayerId](const FPlayerTeamInfo& PlayerInfo)
		{
			return PlayerInfo.PlayerId == PlayerId;
		});

	if (Info)
	{
		OutInfo = *Info;
		return true;
	}

	return false;
}

const TArray<AActor*>& AMD_GameState::GetUnitsInTeam(const EMDTeam& Team) const
{
	if (!AllTeamUnits.Contains(Team))
	{
		static TArray<AActor*> EmptyArray;
		return EmptyArray;
	}

	return AllTeamUnits[Team].AllUnits;
}

TArray<FPlayerTeamInfo> AMD_GameState::GetPlayersInTeam(EMDTeam Team) const
{
	TArray<FPlayerTeamInfo> Result;

	for (const FPlayerTeamInfo& Info : PlayersInfo)
	{
		if (Info.GetTeam() == Team)
		{
			Result.Add(Info);
		}
	}

	return Result;
}

void AMD_GameState::OnRep_PlayersInfo() const
{
	FString NetPrefix = GetWorld()->GetNetMode() == NM_Client ? FString::Printf(TEXT("Client %d"), UE::GetPlayInEditorID()) : TEXT("Server");
	// Логика на клиенте: например, обновить список игроков в HUD
	UE_LOG(LogTemp, Log, TEXT("@@@ [ %s ] OnRep PI Updated. Array size: %d"), *NetPrefix, PlayersInfo.Num());
}

void AMD_GameState::OnRep_MatchStage() const
{
	switch (MatchStage)
	{
		case EMatchStage::Draft:

			// Находим локальный контроллер и говорим ему активировать Draft-абилку
			if (AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				if (AMD_CameraPawn* CamPawn = Cast<AMD_CameraPawn>(PC->GetPawn()))
				{
					CamPawn->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer{MyDotaTags::Ability_ShowDraft});
				}
			}
			break;

		case EMatchStage::PreGame:

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

void AMD_GameState::OnRep_LastChange() const
{
	FString OwnerString = HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT");
	// Этот метод вызывается на ВСЕХ клиентах, когда LastTeamChange реплицируется
	UE_LOG(LogTemp, Log, TEXT("[ %s ]AMD_GameState::OnRep_LastChange - Type: %d, Team: %d, Player: %d, Hero: %d"), *OwnerString, (uint8)LastTeamChange.ChangeType, (uint8)LastTeamChange.Team,
		LastTeamChange.PlayerId, LastTeamChange.HeroId);

	switch (LastTeamChange.ChangeType)
	{
		case EChangeType::PlayerJoined: OnPlayerJoinedTeam.Broadcast(LastTeamChange.PlayerId, LastTeamChange.GetTeam()); break;

		case EChangeType::PlayerLeft: OnPlayerLeftTeam.Broadcast(LastTeamChange.PlayerId, LastTeamChange.GetTeam()); break;

		case EChangeType::HeroSelected: OnPlayerHeroSelected.Broadcast(LastTeamChange.PlayerId, LastTeamChange.HeroId, LastTeamChange.GetTeam()); break;

		case EChangeType::TeamChanged: OnTeamCompositionChanged.Broadcast(LastTeamChange.GetTeam()); break;

		default: break;
	}
}

void AMD_GameState::OnRep_UpdateStageEndTime() const
{
	OnGameTimeChanged.Broadcast();
}

void AMD_GameState::BroadcastTeamChanges(EMDTeam ChangedTeam, int32 ChangedPlayerId, EMDTeam OldTeam)
{
	// Оповещаем об общем изменении состава
	OnTeamCompositionChanged.Broadcast(ChangedTeam);

	// Если есть конкретный игрок, можно добавить специфичные делегаты
	if (ChangedPlayerId != -1)
	{
		if (OldTeam != EMDTeam::None)
		{
			// Игрок покинул старую команду
			OnPlayerLeftTeam.Broadcast(ChangedPlayerId, OldTeam);
		}

		if (ChangedTeam != EMDTeam::None)
		{
			// Игрок присоединился к новой команде
			OnPlayerJoinedTeam.Broadcast(ChangedPlayerId, ChangedTeam);
		}
	}
}

void AMD_GameState::RecordChange(EMDTeam Team, int32 PlayerId, int32 HeroId, EChangeType ChangeType)
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Log, TEXT("AMD_GameState::RecordChange ChangeType = %d"), ChangeType)

	LastTeamChange.SetTeam(Team);
	LastTeamChange.PlayerId = PlayerId;
	LastTeamChange.HeroId = HeroId;
	LastTeamChange.ChangeType = ChangeType;

	// Принудительно помечаем для репликации
	MARK_PROPERTY_DIRTY_FROM_NAME(AMD_GameState, LastTeamChange, this);
}

void AMD_GameState::SetMatchStage(EMatchStage NewStage)
{
	if (HasAuthority())
	{
		MatchStage = NewStage;

		OnRep_MatchStage();
	}
}

bool AMD_GameState::AreAllHeroesSelected() const
{
	if (!HasAuthority()) return false;

	// Проверяем, что у всех игроков HeroId не -1
	return PlayersInfo.FindByPredicate(
			   [](const FPlayerTeamInfo& P)
			   {
				   return P.HeroId == -1;
			   }) == nullptr;
}

bool AMD_GameState::IsHeroAlreadyPicked(const int32 HeroIndex) const
{
	if (HasAuthority())
	{
		return PlayersInfo.FindByPredicate(
				   [HeroIndex](const FPlayerTeamInfo& P)
				   {
					   return P.HeroId == HeroIndex;
				   }) != nullptr;
	}
	return false;
}

void AMD_GameState::RegisterUnit(AActor* Unit)
{
	if (!Unit) return;

	const IFogOfWarTeamInterface* TeamInterface = Cast<const IFogOfWarTeamInterface>(Unit);
	if (!TeamInterface) return;

	FTeamUnits& TeamUnits = AllTeamUnits.FindOrAdd(TeamInterface->GetTeam());
	TeamUnits.AllUnits.AddUnique(Unit);
}

void AMD_GameState::UnregisterUnit(AActor* Unit)
{
	if (!Unit) return;

	const IFogOfWarTeamInterface* TeamInterface = Cast<const IFogOfWarTeamInterface>(Unit);
	if (!TeamInterface) return;

	if (FTeamUnits* TeamUnits = AllTeamUnits.Find(TeamInterface->GetTeam()))
	{
		TeamUnits->AllUnits.Remove(Unit);
	}
}
