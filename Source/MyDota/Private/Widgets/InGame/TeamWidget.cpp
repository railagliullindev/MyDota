// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/TeamWidget.h"

#include "Controllers/MD_PlayerController.h"
#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Subsystems/MD_DataSubsystem.h"
#include "Widgets/InGame/HeroSlotWidget.h"

UTeamWidget::UTeamWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTeamWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Собираем все слоты в массив для удобного доступа по индексу
	AllSlots.Empty(10);

	// Radiant слоты (0-4)
	AllSlots.Add(RadiantSlot_0);
	AllSlots.Add(RadiantSlot_1);
	AllSlots.Add(RadiantSlot_2);
	AllSlots.Add(RadiantSlot_3);
	AllSlots.Add(RadiantSlot_4);

	// Dire слоты (5-9)
	AllSlots.Add(DireSlot_0);
	AllSlots.Add(DireSlot_1);
	AllSlots.Add(DireSlot_2);
	AllSlots.Add(DireSlot_3);
	AllSlots.Add(DireSlot_4);

	// Проверяем, что все слоты найдены
	for (int32 i = 0; i < AllSlots.Num(); i++)
	{
		if (!AllSlots[i])
		{
			UE_LOG(LogTemp, Error, TEXT("MyTeamWidget: Slot %d is null! Check BindWidget in BP"), i);
		}
	}

	InitializeTeamWidget();
}

void UTeamWidget::NativeDestruct()
{
	// Отписываемся от всех делегатов GameState
	if (CachedGameState)
	{
		CachedGameState->OnTeamCompositionChanged.RemoveDynamic(this, &UTeamWidget::HandleTeamCompositionChanged);
		CachedGameState->OnPlayerJoinedTeam.RemoveDynamic(this, &UTeamWidget::HandlePlayerJoinedTeam);
		CachedGameState->OnPlayerLeftTeam.RemoveDynamic(this, &UTeamWidget::HandlePlayerLeftTeam);
		CachedGameState->OnPlayerHeroSelected.RemoveDynamic(this, &UTeamWidget::HandlePlayerHeroSelected);
	}

	// Очищаем таймер
	if (DebounceTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DebounceTimerHandle);
	}

	Super::NativeDestruct();
}

void UTeamWidget::InitializeTeamWidget()
{
	if (bIsInitialized) return;

	UWorld* World = GetWorld();
	if (!World) return;

	CachedGameState = World->GetGameState<AMD_GameState>();
	if (!CachedGameState)
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &UTeamWidget::InitializeTeamWidget);
		World->GetTimerManager().SetTimer(DebounceTimerHandle, TimerDelegate, 0.2f, false);
		return;
	}

	CachedPlayerController = Cast<AMD_PlayerController>(World->GetFirstPlayerController());
	if (CachedPlayerController)
	{
		CachedLocalPlayerState = CachedPlayerController->GetPlayerState<AMD_PlayerState>();
	}

	// Подписываемся на делегаты
	CachedGameState->OnTeamCompositionChanged.AddDynamic(this, &UTeamWidget::HandleTeamCompositionChanged);
	CachedGameState->OnPlayerHeroSelected.AddDynamic(this, &UTeamWidget::HandlePlayerHeroSelected);
	CachedGameState->OnPlayerJoinedTeam.AddDynamic(this, &UTeamWidget::HandlePlayerJoinedTeam);
	CachedGameState->OnPlayerLeftTeam.AddDynamic(this, &UTeamWidget::HandlePlayerLeftTeam);

	// Проверяем, что PlayersInfo пришел
	UE_LOG(LogTemp, Warning, TEXT("Delayed init - PlayersInfo count: %d"), CachedGameState->PlayersInfo.Num());

	// Заполняем карту
	PlayerIdToSlotMap.Empty();

	// Radiant
	TArray<FPlayerTeamInfo> RadiantPlayers = CachedGameState->GetPlayersInTeam(EMDTeam::Radiant);
	for (int32 i = 0; i < RadiantPlayers.Num(); i++)
	{
		PlayerIdToSlotMap.Add(RadiantPlayers[i].PlayerId, i);
		UE_LOG(LogTemp, Log, TEXT("INIT: Mapped Radiant Player %d -> Slot %d"), RadiantPlayers[i].PlayerId, i);
	}

	// Dire
	TArray<FPlayerTeamInfo> DirePlayers = CachedGameState->GetPlayersInTeam(EMDTeam::Dire);
	for (int32 i = 0; i < DirePlayers.Num(); i++)
	{
		int32 SlotIndex = i + 5;
		PlayerIdToSlotMap.Add(DirePlayers[i].PlayerId, SlotIndex);
		UE_LOG(LogTemp, Log, TEXT("INIT: Mapped Dire Player %d -> Slot %d"), DirePlayers[i].PlayerId, SlotIndex);
	}

	RefreshAllSlots();
	bIsInitialized = true;
}

void UTeamWidget::RefreshTeamsDisplay()
{
	if (!CachedGameState) return;

	// Обновляем кэш для обеих команд
	CachedTeamData.FindOrAdd(EMDTeam::Radiant) = CachedGameState->GetPlayersInTeam(EMDTeam::Radiant);
	CachedTeamData.FindOrAdd(EMDTeam::Dire) = CachedGameState->GetPlayersInTeam(EMDTeam::Dire);

	// Обновляем информацию о локальном игроке
	if (CachedPlayerController)
	{
		CachedLocalPlayerState = CachedPlayerController->GetPlayerState<AMD_PlayerState>();
	}

	// Вызываем событие для Blueprint
	OnTeamDataChanged(EMDTeam::Radiant);
	OnTeamDataChanged(EMDTeam::Dire);
}

TArray<FPlayerSlotInfo> UTeamWidget::GetTeamSlots(EMDTeam Team) const
{
	TArray<FPlayerSlotInfo> Result;

	// Получаем актуальные данные команды из кэша
	const TArray<FPlayerTeamInfo>* TeamPlayers = CachedTeamData.Find(Team);
	if (!TeamPlayers)
	{
		return Result;
	}

	// Определяем базовый цвет команды
	FLinearColor BaseTeamColor = (Team == EMDTeam::Radiant) ? RadiantTeamColor : DireTeamColor;

	// Заполняем слоты
	for (int32 i = 0; i < MaxPlayersPerTeam; i++)
	{
		FPlayerSlotInfo SlotInfo;
		SlotInfo.SlotIndex = i;

		// Проверяем, есть ли игрок на этом слоте
		if (i < TeamPlayers->Num())
		{
			// Заполненный слот
			SlotInfo.PlayerInfo = (*TeamPlayers)[i];
			SlotInfo.bIsEmpty = false;
			SlotInfo.SlotColor = BaseTeamColor;

			// Проверяем, локальный ли это игрок
			if (CachedLocalPlayerState && SlotInfo.PlayerInfo.PlayerId == CachedLocalPlayerState->GetPlayerId())
			{
				SlotInfo.bIsLocalPlayer = true;
				// Можно смешать цвета для подсветки
				SlotInfo.SlotColor = FLinearColor::LerpUsingHSV(BaseTeamColor, LocalPlayerHighlightColor, 0.3f);
			}
		}
		else
		{
			// Пустой слот
			SlotInfo.bIsEmpty = true;
			SlotInfo.PlayerInfo.PlayerName = FString::Printf(TEXT("Слот %d"), i + 1);
			SlotInfo.PlayerInfo.SetTeam(Team);
			SlotInfo.PlayerInfo.HeroId = -1;
			SlotInfo.PlayerInfo.PlayerId = -1;
			SlotInfo.SlotColor = EmptySlotColor;
		}

		Result.Add(SlotInfo);
	}

	return Result;
}

TArray<FPlayerTeamInfo> UTeamWidget::GetPlayersInTeam(EMDTeam Team) const
{
	TArray<FPlayerTeamInfo> Result;

	if (!CachedGameState) return Result;

	// Получаем PlayersInfo из GameState
	// Предполагаем, что в AMD_GameState есть метод GetPlayersInTeam
	// Если такого метода нет, добавим его:

	/*
	for (const FPlayerTeamInfo& Info : CachedGameState->PlayersInfo)
	{
		if (Info.Team == Team)
		{
			Result.Add(Info);
		}
	}
	*/

	// Альтернативно, используем кэш
	if (const TArray<FPlayerTeamInfo>* CachedTeam = CachedTeamData.Find(Team))
	{
		Result = *CachedTeam;
	}

	// Дополняем пустыми слотами до MaxPlayersPerTeam для красивого отображения
	while (Result.Num() < MaxPlayersPerTeam)
	{
		FPlayerTeamInfo EmptySlot;
		EmptySlot.PlayerId = -1;
		EmptySlot.PlayerName = FString::Printf(TEXT("Empty Slot %d"), Result.Num() + 1);
		EmptySlot.SetTeam(Team);
		EmptySlot.HeroId = -1;
		Result.Add(EmptySlot);
	}

	return Result;
}

bool UTeamWidget::IsTeamFull(const EMDTeam Team) const
{
	return GetPlayersCount(Team) >= MaxPlayersPerTeam;
}

int32 UTeamWidget::GetPlayersCount(EMDTeam Team) const
{
	if (!CachedGameState) return 0;

	int32 Count = 0;

	// Подсчитываем реальных игроков (не пустые слоты)
	for (const FPlayerTeamInfo& Info : CachedGameState->PlayersInfo)
	{
		if (Info.GetTeam() == Team && Info.PlayerId != -1)
		{
			Count++;
		}
	}

	return Count;
}

UHeroSlotWidget* UTeamWidget::GetSlotByIndex(int32 SlotIndex) const
{
	if (AllSlots.IsValidIndex(SlotIndex))
	{
		return AllSlots[SlotIndex];
	}
	return nullptr;
}

int32 UTeamWidget::FindSlotIndexForPlayer(int32 PlayerId) const
{
	const int32* SlotIndexPtr = PlayerIdToSlotMap.Find(PlayerId);
	return SlotIndexPtr ? *SlotIndexPtr : -1;
}

void UTeamWidget::RefreshAllSlots()
{
	if (!CachedGameState) return;

	// Radiant слоты (0-4)
	TArray<FPlayerTeamInfo> RadiantPlayers = CachedGameState->GetPlayersInTeam(EMDTeam::Radiant);
	for (int32 i = 0; i < MaxPlayersPerTeam; i++)
	{
		UHeroSlotWidget* HeroSlotWidget = GetSlotByIndex(i);
		if (!HeroSlotWidget) continue;

		if (i < RadiantPlayers.Num())
		{
			const FPlayerTeamInfo& PlayerInfo = RadiantPlayers[i];
			bool bIsLocal = (CachedLocalPlayerState && CachedLocalPlayerState->GetPlayerId() == PlayerInfo.PlayerId);
			HeroSlotWidget->UpdateSlot(PlayerInfo, bIsLocal);

			// Добавляем в карту, если там еще нет
			if (!PlayerIdToSlotMap.Contains(PlayerInfo.PlayerId))
			{
				PlayerIdToSlotMap.Add(PlayerInfo.PlayerId, i);
				UE_LOG(LogTemp, Log, TEXT("Refresh: Added Radiant Player %d -> Slot %d"), PlayerInfo.PlayerId, i);
			}
		}
		else
		{
			HeroSlotWidget->ClearSlot();
		}
	}

	// Dire слоты (5-9)
	TArray<FPlayerTeamInfo> DirePlayers = CachedGameState->GetPlayersInTeam(EMDTeam::Dire);
	for (int32 i = 0; i < MaxPlayersPerTeam; i++)
	{
		int32 SlotIndex = i + 5;
		UHeroSlotWidget* HeroSlotWidget = GetSlotByIndex(SlotIndex);
		if (!HeroSlotWidget) continue;

		if (i < DirePlayers.Num())
		{
			const FPlayerTeamInfo& PlayerInfo = DirePlayers[i];
			bool bIsLocal = (CachedLocalPlayerState && CachedLocalPlayerState->GetPlayerId() == PlayerInfo.PlayerId);
			HeroSlotWidget->UpdateSlot(PlayerInfo, bIsLocal);

			// Добавляем в карту, если там еще нет
			if (!PlayerIdToSlotMap.Contains(PlayerInfo.PlayerId))
			{
				PlayerIdToSlotMap.Add(PlayerInfo.PlayerId, SlotIndex);
				UE_LOG(LogTemp, Log, TEXT("Refresh: Added Dire Player %d -> Slot %d"), PlayerInfo.PlayerId, SlotIndex);
			}
		}
		else
		{
			HeroSlotWidget->ClearSlot();
		}
	}

	// Логируем итоговую карту для отладки
	UE_LOG(LogTemp, Log, TEXT("PlayerIdToSlotMap has %d entries after RefreshAllSlots"), PlayerIdToSlotMap.Num());
	for (const auto& Pair : PlayerIdToSlotMap)
	{
		UE_LOG(LogTemp, Log, TEXT("  Map: Player %d -> Slot %d"), Pair.Key, Pair.Value);
	}
}

void UTeamWidget::RefreshTeamSlots(EMDTeam Team)
{
	if (!CachedGameState) return;

	TArray<FPlayerTeamInfo> Players = CachedGameState->GetPlayersInTeam(Team);
	int32 StartIndex = (Team == EMDTeam::Radiant) ? 0 : 5;

	// Очищаем старые записи для этой команды
	for (auto It = PlayerIdToSlotMap.CreateIterator(); It; ++It)
	{
		int32 SlotIndex = It.Value();
		if (SlotIndex >= StartIndex && SlotIndex < StartIndex + MaxPlayersPerTeam)
		{
			It.RemoveCurrent();
		}
	}

	// Заполняем новыми данными
	for (int32 i = 0; i < MaxPlayersPerTeam; i++)
	{
		int32 SlotIndex = StartIndex + i;
		UHeroSlotWidget* HeroSlotWidget = GetSlotByIndex(SlotIndex);
		if (!HeroSlotWidget) continue;

		if (i < Players.Num())
		{
			const FPlayerTeamInfo& PlayerInfo = Players[i];
			bool bIsLocal = (CachedLocalPlayerState && CachedLocalPlayerState->GetPlayerId() == PlayerInfo.PlayerId);
			HeroSlotWidget->UpdateSlot(PlayerInfo, bIsLocal);
			PlayerIdToSlotMap.Add(PlayerInfo.PlayerId, SlotIndex);
		}
		else
		{
			HeroSlotWidget->ClearSlot();
		}
	}
}

void UTeamWidget::HandleTeamCompositionChanged(EMDTeam ChangedTeam)
{
	UE_LOG(LogTemp, Log, TEXT("HandleTeamCompositionChanged: Team %d"), (uint8)ChangedTeam);

	// Используем дебаунс, чтобы не обновлять UI слишком часто
	if (DebounceTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DebounceTimerHandle);
	}

	GetWorld()->GetTimerManager().SetTimer(DebounceTimerHandle,
		FTimerDelegate::CreateLambda(
			[this, ChangedTeam]()
			{
				if (this && CachedGameState)
				{
					RefreshTeamSlots(ChangedTeam);
				}
			}),
		0.05f, // Небольшая задержка для накопления изменений
		false);
}

void UTeamWidget::HandlePlayerJoinedTeam(int32 PlayerId, EMDTeam Team)
{
	UE_LOG(LogTemp, Log, TEXT("HandlePlayerJoinedTeam - Player: %d, Team: %d, Current Map: %d entries"), PlayerId, (uint8)Team, PlayerIdToSlotMap.Num());

	if (CachedGameState)
	{
		TArray<FPlayerTeamInfo> TeamPlayers = CachedGameState->GetPlayersInTeam(Team);

		int32 PlayerIndex = TeamPlayers.IndexOfByPredicate(
			[PlayerId](const FPlayerTeamInfo& Info)
			{
				return Info.PlayerId == PlayerId;
			});

		if (PlayerIndex != INDEX_NONE)
		{
			int32 SlotIndex = (Team == EMDTeam::Radiant) ? PlayerIndex : PlayerIndex + 5;

			// ВАЖНО: Обновляем карту, даже если запись уже есть
			PlayerIdToSlotMap.Add(PlayerId, SlotIndex);

			UHeroSlotWidget* HeroSlotWidget = GetSlotByIndex(SlotIndex);
			if (HeroSlotWidget)
			{
				bool bIsLocal = (CachedLocalPlayerState && CachedLocalPlayerState->GetPlayerId() == PlayerId);
				HeroSlotWidget->UpdateSlot(TeamPlayers[PlayerIndex], bIsLocal);

				UE_LOG(LogTemp, Log, TEXT("  -> Player %d assigned to slot %d (local: %d)"), PlayerId, SlotIndex, bIsLocal);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  -> Player %d not found in team %d players list!"), PlayerId, (uint8)Team);
			RefreshTeamSlots(Team);
		}
	}
}

void UTeamWidget::HandlePlayerLeftTeam(int32 PlayerId, EMDTeam Team)
{
	UE_LOG(LogTemp, Log, TEXT("HandlePlayerLeftTeam - Player: %d, Team: %d"), PlayerId, (uint8)Team);
	RefreshTeamSlots(Team);
}

void UTeamWidget::HandlePlayerHeroSelected(int32 PlayerId, int32 HeroId, EMDTeam Team)
{
	UE_LOG(LogTemp, Log, TEXT("HandlePlayerHeroSelected - Player: %d, Hero: %d, Team: %d"), PlayerId, HeroId, (uint8)Team);

	// 1. Пытаемся найти слот через карту
	int32 SlotIndex = FindSlotIndexForPlayer(PlayerId);

	// 2. Если не нашли в карте, ищем через GameState
	if (SlotIndex == -1 && CachedGameState)
	{
		TArray<FPlayerTeamInfo> TeamPlayers = CachedGameState->GetPlayersInTeam(Team);

		int32 PlayerIndexInTeam = TeamPlayers.IndexOfByPredicate(
			[PlayerId](const FPlayerTeamInfo& Info)
			{
				return Info.PlayerId == PlayerId;
			});

		if (PlayerIndexInTeam != INDEX_NONE)
		{
			SlotIndex = (Team == EMDTeam::Radiant) ? PlayerIndexInTeam : PlayerIndexInTeam + 5;

			// Сохраняем в карту
			PlayerIdToSlotMap.Add(PlayerId, SlotIndex);

			UE_LOG(LogTemp, Log, TEXT("  -> Found via GameState: PlayerIndex=%d, SlotIndex=%d"), PlayerIndexInTeam, SlotIndex);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  -> Player %d not found in team %d!"), PlayerId, (uint8)Team);

			// Возможно, игрок в другой команде? Проверим обе
			TArray<FPlayerTeamInfo> RadiantPlayers = CachedGameState->GetPlayersInTeam(EMDTeam::Radiant);
			TArray<FPlayerTeamInfo> DirePlayers = CachedGameState->GetPlayersInTeam(EMDTeam::Dire);

			for (int32 i = 0; i < RadiantPlayers.Num(); i++)
			{
				if (RadiantPlayers[i].PlayerId == PlayerId)
				{
					SlotIndex = i;
					PlayerIdToSlotMap.Add(PlayerId, SlotIndex);
					Team = EMDTeam::Radiant;
					UE_LOG(LogTemp, Log, TEXT("  -> Found in Radiant instead! Slot %d"), SlotIndex);
					break;
				}
			}

			if (SlotIndex == -1)
			{
				for (int32 i = 0; i < DirePlayers.Num(); i++)
				{
					if (DirePlayers[i].PlayerId == PlayerId)
					{
						SlotIndex = i + 5;
						PlayerIdToSlotMap.Add(PlayerId, SlotIndex);
						Team = EMDTeam::Dire;
						UE_LOG(LogTemp, Log, TEXT("  -> Found in Dire instead! Slot %d"), SlotIndex);
						break;
					}
				}
			}
		}
	}

	if (SlotIndex != -1)
	{
		UHeroSlotWidget* HeroSlotWidget = GetSlotByIndex(SlotIndex);
		if (HeroSlotWidget)
		{
			// Обновляем информацию о герое
			FPlayerTeamInfo UpdatedInfo = HeroSlotWidget->GetPlayerInfo();
			UpdatedInfo.HeroId = HeroId;
			UpdatedInfo.SetTeam(Team);
			UpdatedInfo.PlayerId = PlayerId;

			bool bIsLocal = (CachedLocalPlayerState && CachedLocalPlayerState->GetPlayerId() == PlayerId);
			HeroSlotWidget->UpdateSlot(UpdatedInfo, bIsLocal);

			UE_LOG(LogTemp, Log, TEXT("  -> Updated slot %d with hero %d"), SlotIndex, HeroId);

			if (auto* DB = UMD_DataSubsystem::Get(this))
			{
				auto HeroInfo = DB->GetHeroInfo(HeroId);
				if (HeroInfo.IsValid())
				{
					HeroSlotWidget->SetHeroIcon(HeroInfo.HeroIcon);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("  -> CRITICAL: Could not find slot for player %d!"), PlayerId);
	}
}

void UTeamWidget::OnRep_PlayersInfo()
{
	// Этот метод будет вызываться, когда обновляется PlayersInfo в GameState
	// Для этого нужно добавить делегат в GameState или вызывать этот метод из GameState

	RefreshTeamsDisplay();
}

void UTeamWidget::OnLocalPlayerTeamChanged(EMDTeam NewTeam)
{
	// Обновляем отображение при смене команды локальным игроком
	RefreshTeamsDisplay();

	// Оповещаем подписчиков
	// OnLocalPlayerTeamAssigned.Broadcast(NewTeam, IsTeamFull(NewTeam));
}