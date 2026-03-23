// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "MD_ReplicationGraph.generated.h"

class AMD_CharacterBase;
class AFogOfWarManager;

//=============================================================================
// UReplicationGraphNode_AlwaysRelevant_WithPending
//=============================================================================

/**
 * Узел репликации для акторов, которые всегда релевантны всем клиентам,
 * но требуют обработки отложенных запросов перед репликацией.
 *
 * Используется для акторов из AlwaysRelevantClasses (GameState, PlayerState).
 */
UCLASS()
class MYDOTA_API UReplicationGraphNode_AlwaysRelevant_WithPending : public UReplicationGraphNode_ActorList
{
	GENERATED_BODY()

public:

	/** Вызывается перед репликацией для обработки отложенных акторов и запросов команд */
	UReplicationGraphNode_AlwaysRelevant_WithPending();

protected:

	virtual void PrepareForReplication() override;
};

//=============================================================================
// UReplicationGraphNode_AlwaysRelevant_ForTeam
//=============================================================================

/**
 * Узел репликации для акторов, релевантность которых зависит от команды.
 *
 * Принцип работы:
 * - Свои акторы видны всегда (через TeamConnectionNode своих союзников)
 * - Вражеские акторы видны только через туман войны (проверка IsCellVisible)
 */
UCLASS()
class MYDOTA_API UReplicationGraphNode_AlwaysRelevant_ForTeam : public UReplicationGraphNode_ActorList
{
	GENERATED_BODY()

public:

	/** Собрать списки акторов для конкретного соединения */
	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

protected:

	/** Стандартный сбор списков акторов (без фильтрации по команде) */
	virtual void GatherActorListsForConnectionDefault(const FConnectionGatherActorListParameters& Params);
};

//=============================================================================
// UMD_ConnectionManager
//=============================================================================

/**
 * Расширенный менеджер соединения для ReplicationGraph.
 * Хранит информацию о команде игрока и его pawn для фильтрации репликации.
 */
UCLASS()
class UMD_ConnectionManager : public UNetReplicationGraphConnection
{
	GENERATED_BODY()

public:

	//-----------------------------------------------------------------------
	// Узлы репликации
	//-----------------------------------------------------------------------

	/** Узел для акторов, релевантных только владельцу */
	UPROPERTY()
	UReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantForConnectionNode;

	/** Узел для акторов, релевантных команде */
	UPROPERTY()
	UReplicationGraphNode_AlwaysRelevant_ForTeam* TeamConnectionNode;

	//-----------------------------------------------------------------------
	// Данные соединения
	//-----------------------------------------------------------------------

	/** ID команды игрока (-1 = не назначена) */
	int32 TeamId = -1;

	/** Pawn игрока (используется для проверки видимости) */
	UPROPERTY()
	TWeakObjectPtr<APawn> Pawn = nullptr;
};

//=============================================================================
// FTeamConnectionListMap
//=============================================================================

/**
 * Структура для хранения маппинга команд к соединениям.
 * Расширяет TMap для удобной работы с командами и видимостью.
 */
struct FTeamConnectionListMap : TMap<int32, TArray<UMD_ConnectionManager*>>
{
	//-----------------------------------------------------------------------
	// Основные операции
	//-----------------------------------------------------------------------

	/** Получить массив соединений для команды */
	TArray<UMD_ConnectionManager*>* GetConnectionArrayForTeam(int32 TeamId);

	/** Добавить соединение в команду */
	void AddConnectionToTeam(int32 TeamId, UMD_ConnectionManager* ConnManager);

	/** Удалить соединение из команды */
	void RemoveConnectionFromTeam(int32 TeamId, UMD_ConnectionManager* ConnManager);

	//-----------------------------------------------------------------------
	// Фильтрация видимости
	//-----------------------------------------------------------------------

	/**
	 * Получить соединения вражеской команды, видимые через туман войны.
	 * @param Observer - наблюдатель (свой герой)
	 * @param ObserverTeam - команда наблюдателя
	 * @return Массив соединений врагов, которые видны в данный момент
	 */
	TArray<UMD_ConnectionManager*> GetVisibleConnectionArrayForNonTeam(const APawn* Observer, int32 ObserverTeam);
};

//=============================================================================
// UMD_ReplicationGraph
//=============================================================================

/**
 * Основной класс ReplicationGraph для проекта MyDota.
 * Управляет репликацией с учётом команд и тумана войны.
 *
 * @architecture
 * - AlwaysRelevantClasses: акторы, реплицируемые всем (GameState, PlayerState)
 * - AlwaysRelevantNode: узел для глобально релевантных акторов
 * - TeamConnectionNode: узел для акторов, видимых только своей команде
 *
 * @features
 * - Репликация FogOfWarManager только своей команде
 * - Репликация героев с учётом тумана войны (свои всегда, враги - только в зоне видимости)
 * - Отложенная обработка акторов и запросов команд
 *
 * @flow
 * 1. При подключении игрока создаётся UMD_ConnectionManager
 * 2. При назначении команды вызывается SetTeamForPlayerController
 * 3. При спавне FogManager добавляется в TeamConnectionNode всех соединений своей команды
 * 4. При спавне героя добавляется в TeamConnectionNode своего соединения
 * 5. В GatherActorListsForConnection собираются списки с учётом видимости
 */
UCLASS()
class MYDOTA_API UMD_ReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:

	//-----------------------------------------------------------------------
	// Конструктор и публичные методы
	//-----------------------------------------------------------------------

	UMD_ReplicationGraph();

	/** Установить команду для PlayerController */
	void SetTeamForPlayerController(APlayerController* PlayerController, int32 TeamId);

	/** Обработать отложенные акторы и запросы команд (вызывается перед репликацией) */
	void HandlePendingActorsAndTeamRequests();

protected:

	//-----------------------------------------------------------------------
	// Переопределения UReplicationGraph
	//-----------------------------------------------------------------------

	/** Инициализация глобальных узлов (вызывается один раз) */
	virtual void InitGlobalGraphNodes() override;

	/** Инициализация узлов для нового соединения */
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager) override;

	/** Удаление соединения клиента */
	virtual void RemoveClientConnection(UNetConnection* NetConnection) override;

	/** Сброс состояния при перезагрузке уровня */
	virtual void ResetGameWorldState() override;

	/** Добавление актора в граф репликации */
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;

	/** Удаление актора из графа репликации */
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	//-----------------------------------------------------------------------
	// Внутренние методы
	//-----------------------------------------------------------------------

	/** Обновить команду для соединения (очищает старую, добавляет новую) */
	void UpdateTeamForConnection(UMD_ConnectionManager* ConnManager, int32 NewTeamId);

	/** Получить FogManager для команды (из кэша) */
	AFogOfWarManager* GetFogManager(uint8 TeamID);

	/** Получить ConnectionManager для актора (через NetConnection, Owner или Controller) */
	UMD_ConnectionManager* GetConnectionManagerFromActor(const AActor* Actor);

private:

	//-----------------------------------------------------------------------
	// Данные
	//-----------------------------------------------------------------------

	/** Классы акторов, реплицируемых всем клиентам */
	TArray<UClass*> AlwaysRelevantClasses;

	/** Узел для глобально релевантных акторов */
	UPROPERTY()
	UReplicationGraphNode_AlwaysRelevant_WithPending* AlwaysRelevantNode;

	/** Акторы, ожидающие подключения */
	UPROPERTY()
	TArray<AActor*> PendingConnectionActors;

	/** Отложенные запросы на установку команды */
	TArray<TTuple<int32, APlayerController*>> PendingTeamRequests;

	/** Маппинг команд к соединениям */
	FTeamConnectionListMap TeamConnectionListMap;

	/** Кэш FogManager по ID команды */
	UPROPERTY()
	TMap<uint8, AFogOfWarManager*> CachedFogManagers;

	//-----------------------------------------------------------------------
	// Дружественные классы
	//-----------------------------------------------------------------------

	/** Дружественный класс для доступа к TeamConnectionListMap */
	friend UReplicationGraphNode_AlwaysRelevant_ForTeam;
};