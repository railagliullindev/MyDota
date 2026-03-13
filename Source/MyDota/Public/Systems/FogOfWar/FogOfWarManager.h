// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MyDotaStructTypes.h"
#include "FogOfWarManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnitVisibilityChanged, AActor*, const bool);

/**
 * AFogOfWarManager - Менеджер тумана войны для команд Radiant/Dire
 *
 * @description
 * Этот класс реализует систему тумана войны в стиле Dota 2.
 * Создаются два отдельных экземпляра (для Radiant и Dire), каждый управляет
 * видимостью для своей команды.
 *
 * @architecture
 * - Сервер: рассчитывает видимость, сжимает данные в битовый массив
 * - Клиент: получает сжатые данные, обновляет текстуру для пост-процесс эффекта
 *
 * @features
 * - Система высот (High Ground) - возвышенности блокируют обзор
 * - Статические препятствия (деревья/стены) с тегом "BlockFog"
 * - Плавное затухание тумана на клиенте
 * - Оптимизация через кеширование видимых ячеек
 * - Сжатие данных для репликации (1 бит на ячейку)
 *
 * @howitworks
 * 1. В BeginPlay() инициализируются массивы и запекаются данные уровня
 * 2. Далее для начала просчета, нужно вызвать StartFogOfWar(). Сам по себе не стартует
 * 3. Сервер по таймеру пересчитывает видимость от всех ActiveVisionSources
 * 4. Результат сжимается в CompressedFogData и реплицируется клиентам
 * 5. Клиент обновляет текстуру и пост-процесс материал
 *
 * @usage
 * // Получить менеджер для команды Radiant
 * AFogOfWarManager* FogMgr = AFogOfWarManager::Get(WorldContextObject, (uint8)EMDTeam::Radiant);
 *
 * // Зарегистрировать юнит как источник обзора
 * FogMgr->RegisterSource(MyHero, 1800.f);
 *
 * @note
 * Для работы требуется:
 * - Пост-процесс материал с параметром "FogMask"
 * - Объекты с тегом "BlockFog" для препятствий
 * - GameState с массивом AllUnits для отслеживания видимости юнитов
 *
 * @see
 * - FVisionSource - структура источника видимости
 * - IMDTeamInterface - интерфейс для определения команды
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API AFogOfWarManager : public AActor
{
	GENERATED_BODY()

	/* ============================================================================
	 *  Публичный интерфейс (Доступно всем)
	 * ============================================================================ */
public:

	AFogOfWarManager();

	// --- Глобальный доступ ------------------------------------------------------
	/** Получить менеджер для указанной команды (Thread-safe) */
	static AFogOfWarManager* Get(const UObject* WorldContextObject, const uint8 TeamID = -1);

	// --- Основные методы -------------------------------------------------------
	/** Инициализация менеджера (вызывается в BeginPlay) */
	void InitFogManager();

	/** Управление источниками видимости */
	void RegisterSource(AActor* InActor, const float InRadius);
	void UnRegisterSource(AActor* InActor);

	/** Запуск работы просчета тумана, просто так просчет не начинается (сервер) */
	void StartFogOfWar();
	/** Остановка работы просчета тумана, например поставить на паузу (сервер) */
	void EndFogOfWar();

	// --- Конвертация координат -------------------------------------------------
	/** Получить ячейку по из мировых координат */
	FIntPoint WorldToGrid(const FVector& Location) const;

	/** Получить размеры карты */
	FORCEINLINE FIntPoint GetMapSize() const
	{
		return MapSize;
	};

	FORCEINLINE float GetGridCellSize() const
	{
		return GridCellSize;
	}

	// --- Проверка видимости ----------------------------------------------------
	/** Проверить видимость ячейки (серверная версия) */
	bool IsCellVisible(const FIntPoint& GridPos) const;

	/** Проверить видимость ячейки (клиентская версия с плавностью) */
	bool IsCellVisibleOnClient(const FIntPoint& GridPos) const;

	/** Конвертировать индекс ячейки в мировые координаты */
	FVector GridToWorld(const FIntPoint& GridCoords) const;
	FVector GridToWorld(const int32 Index) const;

	// --- Доступ к материалам ---------------------------------------------------
	UMaterialInstanceDynamic* GetMaterialInstance() const;
	UTexture2D* GetFogTexture() const;

	// --- События --------------------------------------------------------------
	FOnUnitVisibilityChanged OnUnitVisibilityChanged;

	/* ============================================================================
	 *  Свойства (Общие)
	 * ============================================================================ */
public:

	/** Команда, которой принадлежит этот менеджер */
	UPROPERTY(Replicated, VisibleAnywhere)
	EMDTeam AssignedTeamID;

protected:

	/** Размер одной ячейки в юнитах UE */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings", meta = (ClampMin = "25"))
	float GridCellSize = 100.f;

	/** Размер карты в ячейках */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings", meta = (CLampMin = "50"))
	FIntPoint MapSize = FIntPoint(256, 256);

	/* ============================================================================
	 *  Свойства (Только для сервера)
	 * ============================================================================ */
public:

	/** Шаг высоты для уровней террейна */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings|Server", meta = (ClampMin = "1"))
	float TerrainHeightLevel = 128.f;

	/** Тег для объектов, блокирующих обзор */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	FString StaticObstacleTag = "BlockFog";

	/** Тик обновления тумана войны на сервере */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings|Server", meta = (ClampMin = "0.01"))
	float FogUpdateTick = 0.1f;

	/* ============================================================================
	 *  Свойства (Только для клиента)
	 * ============================================================================ */

	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings|Client", meta = (ClampMin = "0.1"))
	float FogFadeSpeed = 3.0f;

	/** Материал тумана войны (Post Process) */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	UMaterialInterface* PostProcessMaterial;

	/** Название параметра PostProcessMaterial, в которую мы передаем FogTexture */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	FName FogMaskParameterName = FName("FogMask");

	/* ============================================================================
	 *  Переопределения виртуальных функций
	 * ============================================================================ */
protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	/* ============================================================================
	 *  Сетевые реплицируемые функции
	 * ============================================================================ */
protected:

	/** Обновление пикселей (Client Side) */
	UFUNCTION()
	void OnRep_CompressedFog();

	/* ============================================================================
	 *  Защищенные данные (доступны в наследниках)
	 * ============================================================================ */
protected:

	/** Массив Actor'ов для расчета видимости */
	UPROPERTY()
	TArray<FVisionSource> ActiveVisionSources;

	/* ============================================================================
	 *  Приватные данные и методы (Только для этого класса)
	 * ============================================================================ */
private:

	// --- Данные для всех сборок ------------------------------------------------
	/** Черновик или рабочая область памяти на сервере */
	TArray<uint8> RawVisibilityData;

	/** Массив высот */
	TArray<uint8> TerrainHeights;

	/** Массив статических препятсвий */
	TArray<bool> StaticObstacles;

	/** Массив для репликации (Биты) */
	UPROPERTY(ReplicatedUsing = OnRep_CompressedFog)
	TArray<uint32> CompressedFogData;

private:

	/** Динамический экземпляр материала */
	UPROPERTY()
	UMaterialInstanceDynamic* FogMaterialInstance;

	/** Таймер для обновления (Client only) */
	FTimerHandle TimerHandle;

	/** Флаг инициализации */
	bool bInitialized = false;

	/** Флаг для принудительного пересчета */
	bool bForceUpdate = false;

	// --- Данные только для клиента ---------------------------------------------

	/** Массив для текстуры на клиенте (RGBA) */
	TArray<FColor> PixelBuffer;

	/** Подготовка текстуры в классе менеджера */
	UPROPERTY()
	UTexture2D* FogTexture;

	/** Специальная структура для обновления регионов текстуры */
	TUniquePtr<FUpdateTextureRegion2D> TextureRegion;

	/** Текущее состояние яркости (0.0f - 1.0f) для каждого пикселя */
	TArray<float> CurrentInterpolatedFog;

	/** Целевое состояние от сервера (распакованное из битов) */
	TArray<float> TargetFogGoal;

	/** Кеш последнего состояния видимости юнитов */
	TMap<int32, bool> LastVisibilityState;

	/** Кэш вражеской каманды, для работы с GS (клиент) */
	EMDTeam EnemyTeamCached = EMDTeam::None;

	/* ============================================================================
	 *  Приватные методы
	 * ============================================================================ */
private:

	// --- Инициализация ---------------------------------------------------------

	/** Запекание данных уровня (только сервер) */
	void BakeLevelData();

	/** Инициализация текстуры маски для шейдера (только клиент) */
	void InitTexture();

	/** Обновление текстуры (только клиент) */
	void UpdateTexture();

	// --- Расчет тумана ---------------------------------------------------------
	/** Рассчитать туман войны (сервер) */
	void CalculateFogOfWar();

	/** Рассчитать туман войны с кэшированием (сервер) */
	void CalculateFogOfWarCached();

	/** Обновить видимость от источника (с кешем) */
	void UpdateLineOfSightCached(FVisionSource& Source);

	// --- Трассировка -----------------------------------------------------------
	/** Трассировка луча (с кеша) */
	void TraceLine(const FIntPoint& Start, const FIntPoint& End, int32 MaxRange, uint8 ViewerHeight, TArray<int32>& OutIndices);

	// --- Хелперы ---------------------------------------------------------------
	/** Получить индекс из координат сетки */
	FORCEINLINE int32 GetIndex(FIntPoint GridCoords) const
	{
		return GridCoords.X + GridCoords.Y * MapSize.X;
	}

	/** Получить высоту ячейки */
	uint8 GetTerrainHeight(const FIntPoint& GridCoords) const;

	/** Обновить состояние видимости юнита (только клиент) */
	void UpdateUnitVisibilityState(AActor* InActor);

	/** Получить команду врагов (зависит от AssignedTeamID) */
	EMDTeam GetCachedEnemyTeam();

	// --- Статики ---------------------------------------------------------------
private:

	// --- Константы видимости ---
	static constexpr uint8 VISIBILITY_VISIBLE = 255;
	static constexpr uint8 VISIBILITY_FOG = 127;
	static constexpr uint8 VISIBILITY_HIDDEN = 0;

	// --- Целевые значения для клиента ---
	static constexpr float TARGET_VISIBLE = 1.0f;
	static constexpr float TARGET_FOG = 0.5f;
	static constexpr float TARGET_HIDDEN = 0.0f;

	// --- Технические константы ---
	static constexpr float BAKE_RAY_HEIGHT = 2000.0f;
	static constexpr float INTERPOLATION_TOLERANCE = 0.001f;
	static constexpr float HALF_CELL_MOVEMENT_FACTOR = 0.5f;
	static constexpr uint8 MAX_HEIGHT_LEVEL = 255;

	// --- Цветовые константы ---
	static constexpr uint8 COLOR_FOG = 127;
	static constexpr uint8 COLOR_ALPHA_OPAQUE = 255;
	static constexpr int32 BYTES_PER_PIXEL = 4; // RGBA

	// --- Битовые операции ---
	static constexpr int32 BITS_PER_WORD = 32;
	static constexpr int32 BIT_INDEX_MASK = 32;

public:

	UFUNCTION()
	static EMDTeam GetEnemyTeam(EMDTeam InTeam);
};