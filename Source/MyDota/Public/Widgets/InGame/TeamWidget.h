// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFrameworks/MD_GameState.h"
#include "TeamWidget.generated.h"

class UHeroSlotWidget;
class AMD_PlayerState;

// Структура для слота игрока в UI
USTRUCT(BlueprintType)
struct FPlayerSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	FPlayerTeamInfo PlayerInfo;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalPlayer = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEmpty = true;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor SlotColor;
};

/**
 * Виджет для отображения состава команд с подпиской на делегаты GameState
 */
UCLASS()
class MYDOTA_API UTeamWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UTeamWidget(const FObjectInitializer& ObjectInitializer);

	// ~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~ End UUserWidget Interface

	/** Инициализация виджета и подписка на делегаты */
	UFUNCTION(BlueprintCallable, Category = "Team Widget")
	void InitializeTeamWidget();

	/** Обновить отображение команд */
	UFUNCTION(BlueprintCallable, Category = "Team Widget")
	void RefreshTeamsDisplay();

	/** Получить информацию о слотах команды для отображения */
	UFUNCTION(BlueprintPure, Category = "Team Widget")
	TArray<FPlayerSlotInfo> GetTeamSlots(EMDTeam Team) const;

	/** Получить игроков в команде */
	UFUNCTION(BlueprintPure, Category = "Team Widget")
	TArray<FPlayerTeamInfo> GetPlayersInTeam(EMDTeam Team) const;

	/** Проверить, полна ли команда */
	UFUNCTION(BlueprintPure, Category = "Team Widget")
	bool IsTeamFull(EMDTeam Team) const;

	/** Получить количество игроков в команде */
	UFUNCTION(BlueprintPure, Category = "Team Widget")
	int32 GetPlayersCount(EMDTeam Team) const;

	/** Максимальное количество игроков в команде */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	int32 MaxPlayersPerTeam = 5;

	/** Получить слот по индексу (0-4 Radiant, 5-9 Dire) */
	UFUNCTION(BlueprintPure, Category = "Team Widget")
	UHeroSlotWidget* GetSlotByIndex(int32 SlotIndex) const;

	/** Получить индекс слота для игрока */
	UFUNCTION(BlueprintPure, Category = "Team Widget")
	int32 FindSlotIndexForPlayer(int32 PlayerId) const;

	/** Обновить все слоты */
	UFUNCTION(BlueprintCallable, Category = "Team Widget")
	void RefreshAllSlots();

	/** Обновить слоты конкретной команды */
	UFUNCTION(BlueprintCallable, Category = "Team Widget")
	void RefreshTeamSlots(EMDTeam Team);

protected:

	// --- BindWidget - эти переменные будут автоматически связаны с виджетами из BP ---

	// Radiant слоты (ожидаем, что в BP они названы именно так)
	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* RadiantSlot_0;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* RadiantSlot_1;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* RadiantSlot_2;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* RadiantSlot_3;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* RadiantSlot_4;

	// Dire слоты
	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* DireSlot_0;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* DireSlot_1;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* DireSlot_2;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* DireSlot_3;

	UPROPERTY(meta = (BindWidget))
	UHeroSlotWidget* DireSlot_4;

	/** События для Blueprint - вызываются при изменении состава */
	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void OnTeamDataChanged(EMDTeam Team);

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void OnPlayerJoined(EMDTeam Team, int32 PlayerId, const FPlayerTeamInfo& PlayerInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void OnPlayerLeft(EMDTeam Team, int32 PlayerId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void OnPlayerSelectedHero(int32 PlayerId, int32 HeroId, EMDTeam Team);

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void OnLocalPlayerTeamAssigned(EMDTeam NewTeam, int32 PlayerSlot);

	// --- Обработчики делегатов GameState ---
	UFUNCTION()
	void HandleTeamCompositionChanged(EMDTeam ChangedTeam);

	UFUNCTION()
	void HandlePlayerJoinedTeam(int32 PlayerId, EMDTeam Team);

	UFUNCTION()
	void HandlePlayerLeftTeam(int32 PlayerId, EMDTeam Team);

	UFUNCTION()
	void HandlePlayerHeroSelected(int32 PlayerId, int32 HeroId, EMDTeam Team);

	/** Обновить конкретную команду в UI */
	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void UpdateRadiantTeamDisplay(const TArray<FPlayerTeamInfo>& RadiantPlayers);

	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void UpdateDireTeamDisplay(const TArray<FPlayerTeamInfo>& DirePlayers);

	/** Обновить слот игрока в команде */
	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void UpdatePlayerSlot(int32 SlotIndex, const FPlayerTeamInfo& PlayerInfo, EMDTeam Team);

	/** Обновить информацию о локальном игроке */
	UFUNCTION(BlueprintImplementableEvent, Category = "Team Widget")
	void UpdateLocalPlayerInfo(EMDTeam LocalTeam, int32 LocalPlayerSlot);

	/** Цвета команд для UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor RadiantTeamColor = FLinearColor(0.2f, 0.8f, 0.2f); // Зеленый

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor DireTeamColor = FLinearColor(0.8f, 0.2f, 0.2f); // Красный

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor EmptySlotColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f); // Полупрозрачный серый

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FLinearColor LocalPlayerHighlightColor = FLinearColor(1.0f, 1.0f, 0.0f, 0.3f);

private:

	/** Обработчики событий GameState */
	UFUNCTION()
	void OnRep_PlayersInfo();

	/** Обработчик изменения команды локального игрока */
	UFUNCTION()
	void OnLocalPlayerTeamChanged(EMDTeam NewTeam);

	/** Массив всех слотов для удобства доступа по индексу */
	UPROPERTY()
	TArray<class UHeroSlotWidget*> AllSlots;

	/** Карта PlayerId -> SlotIndex для быстрого поиска */
	TMap<int32, int32> PlayerIdToSlotMap;

	/** Кэшированные данные для быстрого доступа */
	// UPROPERTY()
	TMap<EMDTeam, TArray<FPlayerTeamInfo>> CachedTeamData;

	/** Ссылки на необходимые объекты */
	UPROPERTY()
	AMD_GameState* CachedGameState;

	UPROPERTY()
	AMD_PlayerState* CachedLocalPlayerState;

	UPROPERTY()
	AMD_PlayerController* CachedPlayerController;

	/** Флаг инициализации */
	bool bIsInitialized = false;

	/** Таймер для дебаунса (на случай частых обновлений) */
	FTimerHandle DebounceTimerHandle;

private:

	int32 LastPlayersInfoVersion = -1;
};
