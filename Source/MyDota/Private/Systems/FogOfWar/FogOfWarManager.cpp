// Rail Agliullin Dev. All Rights Reserved

#include "Systems/FogOfWar/FogOfWarManager.h"

#include "EngineUtils.h"
#include "GameFrameworks/MD_GameState.h"
#include "MyDota/MyDota.h"
#include "Net/UnrealNetwork.h"
#include "Systems/FogOfWar/FogOfWarTeamInterface.h"

AFogOfWarManager::AFogOfWarManager()
{
	bReplicates = true;
	bAlwaysRelevant = false;
	NetPriority = 3.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

AFogOfWarManager* AFogOfWarManager::Get(const UObject* WorldContextObject, const uint8 TeamID)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;

	// UE_LOG(LogFogOfWar, Warning, TEXT("AFogOfWarManager::Get try find team id %hhu"), TeamID);
	for (TActorIterator<AFogOfWarManager> It(World); It; ++It)
	{
		// UE_LOG(LogFogOfWar, Warning, TEXT("AFogOfWarManager::Get It %hhu"), It->AssignedTeamID);
		//   На сервере ищем менеджер конкретной команды
		//   На клиенте AssignedTeamID совпадет только у "своего" менеджера (благодаря IsNetRelevantFor)
		if ((uint8)It->AssignedTeamID == TeamID || TeamID == -1)
		{
			// UE_LOG(LogFogOfWar, Warning, TEXT("AFogOfWarManager::Get Find"));
			return *It;
		}
	}
	return nullptr;
}

void AFogOfWarManager::BeginPlay()
{
	Super::BeginPlay();

	InitFogManager();
}

void AFogOfWarManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AFogOfWarManager::OnRep_CompressedFog()
{
	if (!bInitialized) return;

	// Для проверки: работает ли связка AFogOfWarManager -> Шейдер
	// for(auto& Color : PixelBuffer) Color = FColor::Red;

	for (int32 i = 0; i < PixelBuffer.Num(); ++i)
	{
		const int32 WordIndex = i / BITS_PER_WORD;
		const int32 BitIndex = i % BIT_INDEX_MASK;
		const bool bIsVisible = (CompressedFogData[WordIndex] >> BitIndex) & 1;

		// Текущее состояние в буфере
		const FColor TargetColor = bIsVisible ? FColor::White : FColor(COLOR_FOG, COLOR_FOG, COLOR_FOG, COLOR_ALPHA_OPAQUE);

		// Для плавности можно делать Lerp между старым цветом и новым,
		// но для начала просто записываем:
		PixelBuffer[i] = TargetColor;
		TargetFogGoal[i] = bIsVisible ? TARGET_VISIBLE : TARGET_FOG;
	}

	UpdateTexture();
}

void AFogOfWarManager::TraceLine(const FIntPoint& Start, const FIntPoint& End, int32 MaxRange, uint8 ViewerHeight, TArray<int32>& OutIndices)
{
	// 1. Вычисляем параметры прямой
	int32 x = Start.X;
	int32 y = Start.Y;
	int32 dx = FMath::Abs(End.X - Start.X);
	int32 dy = FMath::Abs(End.Y - Start.Y);
	const int32 x_inc = (End.X > Start.X) ? 1 : -1;
	const int32 y_inc = (End.Y > Start.Y) ? 1 : -1;
	int32 error = dx - dy;
	dx *= 2;
	dy *= 2;

	// Максимальное количество шагов (чтобы не уйти в бесконечный цикл)
	int32 n = 1 + (dx / 2) + (dy / 2);
	const int32 MaxRangeSq = MaxRange * MaxRange;

	for (; n > 0; --n)
	{
		// 2. Проверка границ сетки (защита от краша)
		if (x < 0 || x >= MapSize.X || y < 0 || y >= MapSize.Y) break;

		int32 Index = x + y * MapSize.X;

		// 3. Проверка дистанции (чтобы обзор был круглым, а не квадратным)
		int32 CurDistSq = (x - Start.X) * (x - Start.X) + (y - Start.Y) * (y - Start.Y);
		if (CurDistSq > MaxRangeSq) break;

		// 4. Логика препятствий (Деревья / Стены)
		if (StaticObstacles[Index])
		{
			// В Dota мы видим само дерево, но не то, что ЗА ним
			OutIndices.AddUnique(Index);
			break;
		}

		// 5. Логика High Ground
		if (TerrainHeights[Index] > ViewerHeight)
		{
			// Видим только склон (границу возвышенности), но не саму гору
			OutIndices.AddUnique(Index);
			break;
		}

		// 6. Если всё чисто — добавляем ячейку в кеш видимости
		OutIndices.AddUnique(Index);

		// 7. Шаг алгоритма Брезенхема
		if (error > 0)
		{
			x += x_inc;
			error -= dy;
		}
		else
		{
			y += y_inc;
			error += dx;
		}
	}
}

void AFogOfWarManager::UpdateTexture()
{
	if (!FogTexture) return;

	// Быстрая заливка буфера в видеопамять
	const auto RegionData = TextureRegion.Get();
	const auto DataPtr = PixelBuffer.GetData();

	FogTexture->UpdateTextureRegions(0, 1, RegionData, MapSize.X * BYTES_PER_PIXEL, BYTES_PER_PIXEL, (uint8*)DataPtr);
}

void AFogOfWarManager::BakeLevelData()
{
	const int32 TotalCells = MapSize.X * MapSize.Y;

	if (TotalCells <= 0)
	{
		UE_LOG(LogFogOfWar, Error, TEXT("FogManager [Server]: MapSize is Zero!"));
		return;
	}

	// Инициализируем наш "черновик"
	RawVisibilityData.SetNumZeroed(TotalCells);

	// Инициализируем другие массивы
	TerrainHeights.SetNumUninitialized(TotalCells);
	StaticObstacles.SetNumUninitialized(TotalCells);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	for (int32 y = 0; y < MapSize.Y; y++)
	{
		for (int32 x = 0; x < MapSize.X; x++)
		{
			const int32 Index = x + y * MapSize.X;

			// Переводим координаты сетки в мировые координаты
			FVector CellWorldPos = GridToWorld(FIntPoint(x, y));
			FVector RayStart = CellWorldPos + FVector(0, 0, BAKE_RAY_HEIGHT); // С неба
			FVector RayEnd = CellWorldPos - FVector(0, 0, BAKE_RAY_HEIGHT);	  // В пол

			FHitResult Hit;
			// Трейсим, чтобы найти землю (Landscape) и объекты (Trees/Walls)
			if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic, QueryParams))
			{
				// 1. Записываем высоту (делим на шаг высоты Dota, например, 128 юнитов)
				// Это превратит 0, 128, 256 в уровни 0, 1, 2
				TerrainHeights[Index] = FMath::Clamp(FMath::FloorToInt(Hit.Location.Z / TerrainHeightLevel), 0, MAX_HEIGHT_LEVEL);

				// 2. Проверяем, является ли объект препятствием
				// Можно проверять по Tag или по Actor Class (например, деревья)
				if (Hit.GetActor() && Hit.GetActor()->ActorHasTag(*StaticObstacleTag))
				{
					StaticObstacles[Index] = true;
				}
				else
				{
					StaticObstacles[Index] = false;
				}
			}
		}
	}
}

uint8 AFogOfWarManager::GetTerrainHeight(const FIntPoint& GridCoords) const
{
	if (GridCoords.X >= 0 && GridCoords.X < MapSize.X && GridCoords.Y >= 0 && GridCoords.Y < MapSize.Y)
	{
		return TerrainHeights[GetIndex(GridCoords)];
	}
	return 0;
}

void AFogOfWarManager::UpdateUnitVisibilityState(AActor* InActor)
{
	const FIntPoint GridPos = WorldToGrid(InActor->GetActorLocation());
	const bool bIsVisible = IsCellVisibleOnClient(GridPos);
	const int32 Id = InActor->GetUniqueID();

	if (!LastVisibilityState.Contains(Id) || LastVisibilityState[Id] != bIsVisible)
	{
		LastVisibilityState.Emplace(Id, bIsVisible);
		OnUnitVisibilityChanged.Broadcast(InActor, bIsVisible);
	}
}

EMDTeam AFogOfWarManager::GetCachedEnemyTeam()
{
	if (EnemyTeamCached == EMDTeam::None && AssignedTeamID != EMDTeam::None)
	{
		EnemyTeamCached = (AssignedTeamID == EMDTeam::Radiant) ? EMDTeam::Dire : EMDTeam::Radiant;
	}

	if (EnemyTeamCached == EMDTeam::None)
	{
		UE_LOG(LogFogOfWar, Error, TEXT("GetCachedEnemyTeam: AssignedTeamID is None"));
	}

	return EnemyTeamCached;
}

EMDTeam AFogOfWarManager::GetEnemyTeam(const EMDTeam InTeam)
{
	switch (InTeam)
	{
		case EMDTeam::Radiant: return EMDTeam::Dire;
		case EMDTeam::Dire: return EMDTeam::Radiant;
		default: return EMDTeam::None;
	}
}

FIntPoint AFogOfWarManager::WorldToGrid(const FVector& Location) const
{
	int32 GridX = FMath::FloorToInt(Location.X / GridCellSize) + (MapSize.X / 2);
	int32 GridY = FMath::FloorToInt(Location.Y / GridCellSize) + (MapSize.Y / 2);

	GridX = FMath::Clamp(GridX, 0, MapSize.X - 1);
	GridY = FMath::Clamp(GridY, 0, MapSize.Y - 1);

	return FIntPoint(GridX, GridY);
}

FVector AFogOfWarManager::GridToWorld(const int32 Index) const
{
	if (!RawVisibilityData.IsValidIndex(Index)) return FVector::ZeroVector;

	// 1. Из индекса в координаты сетки
	const int32 GridX = Index % MapSize.X;
	const int32 GridY = Index / MapSize.X;

	// 2. Из сетки в мировые координаты (относительно центра менеджера)
	const float WorldX = (GridX - (MapSize.X / 2.0f)) * GridCellSize;
	const float WorldY = (GridY - (MapSize.Y / 2.0f)) * GridCellSize;

	// 3. Учитываем позицию самого менеджера в мире
	const FVector ManagerLoc = GetActorLocation();

	// Z берем либо из запеченных высот, либо из позиции менеджера
	float WorldZ = ManagerLoc.Z;
	if (TerrainHeights.IsValidIndex(Index))
	{
		// Помнишь, мы делили на 128 при запекании? Теперь умножаем обратно.
		WorldZ = TerrainHeights[Index] * TerrainHeightLevel;
	}

	return FVector(WorldX + ManagerLoc.X, WorldY + ManagerLoc.Y, WorldZ);
}

bool AFogOfWarManager::IsCellVisible(const FIntPoint& GridPos) const
{
	const int32 Index = GridPos.X + GridPos.Y * MapSize.X;
	if (RawVisibilityData.IsValidIndex(Index))
	{

		return RawVisibilityData[Index] == VISIBILITY_VISIBLE;
	}
	return false;
}

bool AFogOfWarManager::IsCellVisibleOnClient(const FIntPoint& GridPos) const
{
	const int32 Index = GridPos.X + GridPos.Y * MapSize.X;
	if (TargetFogGoal.IsValidIndex(Index))
	{
		return FMath::IsNearlyEqual(TargetFogGoal[Index], TARGET_VISIBLE, INTERPOLATION_TOLERANCE);
	}
	return false;
}

UMaterialInstanceDynamic* AFogOfWarManager::GetMaterialInstance() const
{
	return FogMaterialInstance;
}

UTexture2D* AFogOfWarManager::GetFogTexture() const
{
	return FogTexture;
};

FVector AFogOfWarManager::GridToWorld(const FIntPoint& GridCoords) const
{
	FVector WorldPos;
	WorldPos.X = (GridCoords.X - MapSize.X / 2.0f) * GridCellSize;
	WorldPos.Y = (GridCoords.Y - MapSize.Y / 2.0f) * GridCellSize;
	WorldPos.Z = 0; // Z определится трейсом
	return WorldPos + GetActorLocation();
}

bool AFogOfWarManager::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (const IFogOfWarTeamInterface* ViewerTeam = Cast<IFogOfWarTeamInterface>(RealViewer))
	{
		return ViewerTeam->GetTeam() == this->AssignedTeamID;
	}
	return false;
}

void AFogOfWarManager::CalculateFogOfWar()
{
	// 1. Проверка безопасности: сервер и наличие данных
	if (!HasAuthority() || RawVisibilityData.Num() == 0) return;

	// 2. "Забывание" (Fade): переводим текущий обзор в туман войны
	// Ячейки VISIBILITY_VISIBLE (видимо) становятся VISIBILITY_FOG (туман), VISIBILITY_HIDDEN (чернота) остается VISIBILITY_HIDDEN.
	for (int32 i = 0; i < RawVisibilityData.Num(); ++i)
	{
		if (RawVisibilityData[i] == VISIBILITY_VISIBLE)
		{
			RawVisibilityData[i] = VISIBILITY_FOG;
		}
	}

	// 3. Обновление обзора от всех живых источников
	// Допустим, у тебя есть список зарегистрированных юнитов
	for (FVisionSource& Source : ActiveVisionSources)
	{
		if (IsValid(Source.SourceActor))
		{
			UpdateLineOfSightCached(Source);
		}
	}

	// 4. Сжатие данных в биты для репликации
	// Это автоматически вызовет OnRep_CompressedFog у клиентов
	for (int32 i = 0; i < RawVisibilityData.Num(); ++i)
	{
		const int32 WordIndex = i / BITS_PER_WORD;
		const int32 BitIndex = i % BIT_INDEX_MASK;

		const bool bIsVisible = (RawVisibilityData[i] == VISIBILITY_VISIBLE);

		if (bIsVisible) CompressedFogData[WordIndex] |= (1 << BitIndex);
		else CompressedFogData[WordIndex] &= ~(1 << BitIndex);
	}

	// ВАЖНО: Принудительно помечаем массив для репликации,
	// так как TArray внутри не всегда триггерит репликацию при изменении элементов
	MARK_PROPERTY_DIRTY_FROM_NAME(AFogOfWarManager, CompressedFogData, this);
}

void AFogOfWarManager::CalculateFogOfWarCached()
{
	// 1. Проверка безопасности: сервер и наличие данных
	if (!HasAuthority() || RawVisibilityData.Num() == 0) return;

	bool bAnythingChanged = bForceUpdate;

	// 2. Проверяем, кто из юнитов сдвинулся хотя бы на половину ячейки
	const float Distance = GridCellSize * HALF_CELL_MOVEMENT_FACTOR; // MapSize.X * 0.5f;
	for (FVisionSource& Source : ActiveVisionSources)
	{
		if (!IsValid(Source.SourceActor)) continue;

		FVector CurrentLoc = Source.SourceActor->GetActorLocation();
		uint8 CurrentHeight = GetTerrainHeight(WorldToGrid(CurrentLoc));

		// Если сдвинулся больше чем на 1/2 ячейки (50см при Grid=100) или сменил высоту
		if (FVector::DistSquared(CurrentLoc, Source.LastLocation) > FMath::Square(Distance) || CurrentHeight != Source.LastViewerHeight)
		{
			Source.bIsDirty = true;
			Source.LastLocation = CurrentLoc;
			Source.LastViewerHeight = CurrentHeight;
			bAnythingChanged = true;
		}
	}

	// 3. Если ничего не изменилось - экономим CPU и не шлем пакеты
	if (!bAnythingChanged) return;

	CalculateFogOfWar();

	bForceUpdate = false;
}

void AFogOfWarManager::UpdateLineOfSightCached(FVisionSource& Source)
{
	if (Source.bIsDirty)
	{
		Source.VisibleIndices.Empty();
		FIntPoint Center = WorldToGrid(Source.LastLocation);
		int32 RangeCells = FMath::RoundToInt(Source.Radius / GridCellSize);

		// Пускаем лучи Брезенхема по периметру
		for (int32 x = Center.X - RangeCells; x <= Center.X + RangeCells; x++)
		{
			TraceLine(Center, FIntPoint(x, Center.Y - RangeCells), RangeCells, Source.LastViewerHeight, Source.VisibleIndices);
			TraceLine(Center, FIntPoint(x, Center.Y + RangeCells), RangeCells, Source.LastViewerHeight, Source.VisibleIndices);
		}
		for (int32 y = Center.Y - RangeCells; y <= Center.Y + RangeCells; y++)
		{
			TraceLine(Center, FIntPoint(Center.X - RangeCells, y), RangeCells, Source.LastViewerHeight, Source.VisibleIndices);
			TraceLine(Center, FIntPoint(Center.X + RangeCells, y), RangeCells, Source.LastViewerHeight, Source.VisibleIndices);
		}
		Source.bIsDirty = false;
	}
	// Применяем кеш (даже если он не dirty, мы должны пометить ячейки как видимые в этом кадре)
	for (const auto Index : Source.VisibleIndices)
	{
		RawVisibilityData[Index] = VISIBILITY_VISIBLE;
	}
}

void AFogOfWarManager::RegisterSource(AActor* InActor, const float InRadius)
{
	if (InActor)
	{
		FVisionSource NewSource;
		NewSource.SourceActor = InActor;
		NewSource.Radius = InRadius;
		ActiveVisionSources.Add(NewSource);
	}
}

void AFogOfWarManager::UnRegisterSource(AActor* InActor)
{
	if (InActor)
	{
		ActiveVisionSources.RemoveAll(
			[InActor](const FVisionSource& Source)
			{
				return Source.SourceActor == InActor;
			});
	}
}

void AFogOfWarManager::StartFogOfWar()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AFogOfWarManager::CalculateFogOfWarCached, FogUpdateTick, true);
	}
}

void AFogOfWarManager::EndFogOfWar()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void AFogOfWarManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Реплицируем только клиентам одной команды
	// В Dota сервер знает всё, а клиент — только свой туман.
	DOREPLIFETIME_CONDITION(AFogOfWarManager, CompressedFogData, COND_SkipOwner);
}

void AFogOfWarManager::Tick(float DeltaSeconds)
{
	if (!HasAuthority())
	{
		bool bChanged = false;

		for (int32 i = 0; i < CurrentInterpolatedFog.Num(); ++i)
		{
			const float Current = CurrentInterpolatedFog[i];
			const float Target = TargetFogGoal[i];

			if (!FMath::IsNearlyEqual(Current, Target, INTERPOLATION_TOLERANCE))
			{
				// FInterpTo делает движение плавным (зависит от FogFadeSpeed)
				CurrentInterpolatedFog[i] = FMath::FInterpTo(Current, Target, DeltaSeconds, FogFadeSpeed);
				bChanged = true;
			}
		}

		// Если хоть один пиксель изменился — пушим в текстуру
		if (bChanged)
		{
			UpdateTexture();
		}

		// Проверка всех AllUnits
		if (AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>())
		{
			const TArray<AActor*>& Units = GS->GetUnitsInTeam(GetCachedEnemyTeam());
			for (const auto Unit : Units)
			{
				if (Unit)
				{
					UpdateUnitVisibilityState(Unit);
				}
			}
		}
	}
}

void AFogOfWarManager::InitFogManager()
{
	const FString RoleString = HasAuthority() ? "Server" : "Client";

	// 1. Защита от нулевого размера
	if (MapSize.X <= 0 || MapSize.Y <= 0)
	{
		UE_LOG(LogFogOfWar, Error, TEXT("FogManager [%s]: MapSize is Invalid (%d, %d)!"), *RoleString, MapSize.X, MapSize.Y);
		return;
	}

	const int32 TotalCells = MapSize.X * MapSize.Y;
	UE_LOG(LogFogOfWar, Log, TEXT("FogManager [%s]: Initializing for %d cells..."), *RoleString, TotalCells);

	// 2. Инициализация статических данных (Нужны и серверу, и клиенту для рейкастов/логики)
	TerrainHeights.Empty();
	TerrainHeights.SetNumZeroed(TotalCells);

	StaticObstacles.Empty();
	StaticObstacles.SetNumZeroed(TotalCells);

	CurrentInterpolatedFog.Empty();
	CurrentInterpolatedFog.SetNumZeroed(TotalCells);

	TargetFogGoal.Empty();
	TargetFogGoal.SetNumZeroed(TotalCells);

	// 3. Инициализация серверных данных
	if (HasAuthority()) // Только на сервере
	{
		RawVisibilityData.Empty();
		RawVisibilityData.SetNumZeroed(TotalCells);

		// Размер битового массива: количество ячеек / 32 (округляем вверх)
		const int32 BitArraySize = FMath::DivideAndRoundUp(TotalCells, BITS_PER_WORD);
		CompressedFogData.Empty();
		CompressedFogData.SetNumZeroed(BitArraySize);

		// Теперь вызываем запекание ландшафта (рейкасты)
		BakeLevelData();
	}

	// 4. Инициализация клиентских данных (Визуал)
	if (GetNetMode() != NM_DedicatedServer)
	{
		PixelBuffer.Empty();
		PixelBuffer.SetNumUninitialized(TotalCells);

		// Здесь же вызываем создание текстуры
		InitTexture();
	}

	UE_LOG(LogFogOfWar, Log, TEXT("FogManager [%s]: Initialization Complete."), *RoleString);
	bInitialized = true;
}

void AFogOfWarManager::InitTexture()
{
	FogTexture = UTexture2D::CreateTransient(MapSize.X, MapSize.Y, PF_B8G8R8A8);
	FogTexture->CompressionSettings = TC_VectorDisplacementmap; // Отключаем компрессию
	FogTexture->SRGB = false;
	FogTexture->Filter = TF_Bilinear; // Мягкая фильтрация даст легкий размыв
	FogTexture->UpdateResource();

	PixelBuffer.SetNumUninitialized(MapSize.X * MapSize.Y);

	TextureRegion = MakeUnique<FUpdateTextureRegion2D>(0, 0, 0, 0, MapSize.X, MapSize.Y);

	if (PostProcessMaterial)
	{
		// 1. Создаем динамический экземпляр
		FogMaterialInstance = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);

		// 2. Сразу передаем нашу текстуру в параметр
		FogMaterialInstance->SetTextureParameterValue(FogMaskParameterName, FogTexture);

		// 3. Добавляем его в настройки глобального PostProcess
		// Для этого ищем на сцене PostProcessVolume или создаем свой
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
			APostProcessVolume* PPVolume = *It;
			if (PPVolume->bUnbound) // Ищем глобальный (бесконечный) объем
			{
				PPVolume->AddOrUpdateBlendable(FogMaterialInstance, 1.0f);
				break;
			}
		}
	}
}

// Только для отладки
/*const TCHAR* NetModeStr = GetNetMode() == NM_Client			   ? TEXT("Client")
						  : GetNetMode() == NM_ListenServer	   ? TEXT("ListenServer")
						  : GetNetMode() == NM_Standalone	   ? TEXT("Standalone")
						  : GetNetMode() == NM_DedicatedServer ? TEXT("Server")
															   : TEXT("Unknown");

UE_LOG(LogFogOfWar, Log, TEXT("FogManager on %s, HasAuthority: %d"), NetModeStr, HasAuthority());*/