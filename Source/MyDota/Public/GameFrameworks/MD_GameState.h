// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameModes/MD_GameMode.h"
#include "MD_GameState.generated.h"

enum class EMDTeam : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayersInfoChanged);

// Делегаты для оповещения об изменениях
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamCompositionChanged, EMDTeam, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinedTeam, int32, PlayerId, EMDTeam, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeftTeam, int32, PlayerId, EMDTeam, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerHeroSelected, int32, PlayerId, int32, HeroId, EMDTeam, Team);

USTRUCT()
struct FTeamUnits
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AActor*> AllUnits;

	FTeamUnits()
		: AllUnits()
	{
	}
};
struct FPlayerTeamInfo;
class AMD_CharacterBase;
/**
 *
 */
UCLASS()
class MYDOTA_API AMD_GameState : public AGameState
{
	GENERATED_BODY()

public:

	AMD_GameState();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Количество игроков в команде */
	UFUNCTION(BlueprintPure, Category = "Teams")
	int32 GetPlayersCountInTeam(const EMDTeam Team) const;

	/** Полна ли команда */
	UFUNCTION(BlueprintPure, Category = "Teams")
	bool IsTeamFull(const EMDTeam Team) const;

	UFUNCTION(BlueprintPure, Category = "Teams")
	bool CanJoinTeam(EMDTeam Team) const;

	/** Получить команду с наименьшим количеством игроков */
	UFUNCTION(BlueprintPure, Category = "Teams")
	EMDTeam GetSmallestTeam() const;

	// --- Методы для управления составом команд ---

	/** Обновить команду игрока */
	void UpdatePlayerTeam(APlayerState* PlayerState, EMDTeam NewTeam);
	/** Обновить выбранного героя */
	void UpdatePlayerHero(APlayerState* PlayerState, int32 NewHeroId);
	/** Удалить игрока (при выходе) */
	void RemovePlayer(APlayerState* PlayerState);

	/** Получить информацию об игроке по PlayerId */
	UFUNCTION(BlueprintPure, Category = "Teams")
	bool GetPlayerInfo(int32 PlayerId, FPlayerTeamInfo& OutInfo) const;

	// Для совместимости со старым кодом, но лучше использовать новые методы
	void RegisterNewPlayer(const int32 PlayerId, const int32 TeamId); /* Deprecated */

	void RegisterHeroSelection(const int32 InPlayerId, const int32 HeroId); /* Deprecated */

	// Установка стадии матча
	void SetMatchStage(EMatchStage NewStage);

	/** Проверить, все ли выбрали героев */
	UFUNCTION(BlueprintPure, Category = "Draft")
	bool AreAllHeroesSelected() const;

	/** Проверить, выбран ли уже герой */
	UFUNCTION(BlueprintPure, Category = "Draft")
	bool IsHeroAlreadyPicked(const int32 HeroIndex) const;

	void RegisterUnit(AActor* Unit);
	void UnregisterUnit(AActor* Unit);

	/** Стадия матча */
	UPROPERTY(ReplicatedUsing = OnRep_MatchStage, BlueprintReadOnly, Category = "MatchStage")
	EMatchStage MatchStage;

	/** Получить всех юнитов команды */
	UFUNCTION(BlueprintCallable, Category = "Team Units")
	const TArray<AActor*>& GetUnitsInTeam(const EMDTeam& Team) const;

	// Делегат для оповещения об изменении состава команд
	UPROPERTY(BlueprintAssignable)
	FOnPlayersInfoChanged OnPlayersInfoChanged;

	/** Получить всех игроков команды */
	UFUNCTION(BlueprintPure, Category = "Teams")
	TArray<FPlayerTeamInfo> GetPlayersInTeam(EMDTeam Team) const;

	// --- Делегаты для UI ---
	/** Вызывается при любом изменении состава любой команды */
	UPROPERTY(BlueprintAssignable)
	FOnTeamCompositionChanged OnTeamCompositionChanged;

	/** Вызывается когда игрок присоединяется к команде */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerJoinedTeam OnPlayerJoinedTeam;

	/** Вызывается когда игрок покидает команду */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerLeftTeam OnPlayerLeftTeam;

	/** Вызывается когда игрок выбирает героя */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerHeroSelected OnPlayerHeroSelected;

	// Вспомогательная структура для репликации последнего изменения
	UPROPERTY(ReplicatedUsing = OnRep_LastChange)
	FPlayerTeamInfo LastTeamChange;

protected:

	// --- OnRep функции ---

	UFUNCTION()
	void OnRep_MatchStage();

	UFUNCTION()
	void OnRep_LastChange();

	// Конфигурация
	UPROPERTY(EditDefaultsOnly, Category = "Match Setup")
	int32 MaxPlayersPerTeam = 5;

	UPROPERTY()
	TMap<EMDTeam, FTeamUnits> AllTeamUnits;

private:

	// Вспомогательный метод для вызова всех делегатов при изменении
	void BroadcastTeamChanges(EMDTeam ChangedTeam, int32 ChangedPlayerId = -1, EMDTeam OldTeam = EMDTeam::None);

	// Метод для записи изменения и принудительной репликации
	void RecordChange(EMDTeam Team, int32 PlayerId, int32 HeroId, EChangeType ChangeType);

public:

	// Реплицируемый массив информации об игроках
	UPROPERTY(ReplicatedUsing = OnRep_PlayersInfo)
	TArray<FPlayerTeamInfo> PlayersInfo;

	UFUNCTION()
	void OnRep_PlayersInfo();
};
