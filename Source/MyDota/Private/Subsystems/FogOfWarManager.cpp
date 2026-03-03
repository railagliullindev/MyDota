// Rail Agliullin Dev. All Rights Reserved

#include "Subsystems/FogOfWarManager.h"

#include "EngineUtils.h"
#include "Interfaces/MDTeamInterface.h"
#include "MyDota/MyDota.h"
#include "Net/UnrealNetwork.h"

AFogOfWarManager::AFogOfWarManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = true;
	bAlwaysRelevant = true; // Важно! Теперь релевантность считаем сами
	NetPriority = 3.0f;
}

AFogOfWarManager* AFogOfWarManager::Get(const UObject* WorldContextObject, uint8 TeamID)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;

	for (TActorIterator<AFogOfWarManager> It(World); It; ++It)
	{
		// На сервере ищем менеджер конкретной команды
		// На клиенте AssignedTeamID совпадет только у "своего" менеджера (благодаря IsNetRelevantFor)
		if ((uint8)It->AssignedTeamID == TeamID || TeamID == -1)
		{
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

	PixelBuffer[0] = FColor::Red;

	// Для проверки: работает ли связка AFogOfWarManager -> Шейдер
	// for(auto& Color : PixelBuffer) Color = FColor::Red;

	for (int32 i = 0; i < PixelBuffer.Num(); ++i)
	{
		const int32 WordIndex = i / 32;
		const int32 BitIndex = i % 32;
		const bool bIsVisible = (CompressedFogData[WordIndex] >> BitIndex) & 1;

		// Текущее состояние в буфере
		const FColor TargetColor = bIsVisible ? FColor::White : FColor(127, 127, 127, 255);

		// Для плавности можно делать Lerp между старым цветом и новым,
		// но для начала просто записываем:
		PixelBuffer[i] = TargetColor;
		TargetFogGoal[i] = bIsVisible ? 1.f : 0.5f;
	}

	UpdateTexture();
}

void AFogOfWarManager::TraceLine(FIntPoint Start, FIntPoint End, int32 MaxRange, uint8 ViewerHeight)
{
	// Стандартный алгоритм Брезенхема для прохода по сетке
	int32 dx = FMath::Abs(End.X - Start.X);
	int32 dy = FMath::Abs(End.Y - Start.Y);
	int32 x = Start.X;
	int32 y = Start.Y;
	int32 n = 1 + dx + dy;
	int32 x_inc = (End.X > Start.X) ? 1 : -1;
	int32 y_inc = (End.Y > Start.Y) ? 1 : -1;
	int32 error = dx - dy;
	dx *= 2;
	dy *= 2;

	for (; n > 0; --n)
	{
		int32 Index = x + y * MapSize.X;

		// Проверка на препятствие (дерево или стена)
		if (StaticObstacles[Index]) break;

		// Dota-logic: Хайграунд блокирует обзор, если ты ниже
		if (TerrainHeights[Index] > ViewerHeight) break;

		// Помечаем ячейку как видимую
		RawVisibilityData[Index] = 255;

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

		// Ограничение по радиусу (дистанция в ячейках)
		if (FVector2D::DistSquared(FVector2D(x, y), FVector2D(Start)) > FMath::Square(MaxRange)) break;
	}
}

void AFogOfWarManager::UpdateTexture()
{
	if (!FogTexture) return;

	// Быстрая заливка буфера в видеопамять
	const auto RegionData = TextureRegion;
	const auto DataPtr = PixelBuffer.GetData();

	FogTexture->UpdateTextureRegions(0, 1, RegionData, MapSize.X * 4, 4, (uint8*)DataPtr);
}

void AFogOfWarManager::BakeLevelData()
{
	int32 TotalCells = MapSize.X * MapSize.Y;

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
			int32 Index = x + y * MapSize.X;

			// Переводим координаты сетки в мировые координаты
			FVector CellWorldPos = GridToWorld(FIntPoint(x, y));
			FVector RayStart = CellWorldPos + FVector(0, 0, 2000.f); // С неба
			FVector RayEnd = CellWorldPos - FVector(0, 0, 2000.f);	 // В пол

			FHitResult Hit;
			// Трейсим, чтобы найти землю (Landscape) и объекты (Trees/Walls)
			if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic, QueryParams))
			{
				// 1. Записываем высоту (делим на шаг высоты Dota, например, 128 юнитов)
				// Это превратит 0, 128, 256 в уровни 0, 1, 2
				TerrainHeights[Index] = FMath::Clamp(FMath::FloorToInt(Hit.Location.Z / TerrainHeightLevel), 0, 255);

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

uint8 AFogOfWarManager::GetTerrainHeight(FIntPoint GridCoords) const
{
	if (GridCoords.X >= 0 && GridCoords.X < MapSize.X && GridCoords.Y >= 0 && GridCoords.Y < MapSize.Y)
	{
		return TerrainHeights[GetIndex(GridCoords)];
	}
	return 0;
}

FIntPoint AFogOfWarManager::WorldToGrid(FVector Location) const
{
	int32 GridX = FMath::FloorToInt(Location.X / GridCellSize) + (MapSize.X / 2);
	int32 GridY = FMath::FloorToInt(Location.Y / GridCellSize) + (MapSize.Y / 2);

	GridX = FMath::Clamp(GridX, 0, MapSize.X - 1);
	GridY = FMath::Clamp(GridY, 0, MapSize.Y - 1);

	return FIntPoint(GridX, GridY);
}

FVector AFogOfWarManager::GridToWorld(FIntPoint GridCoords) const
{
	FVector WorldPos;
	WorldPos.X = (GridCoords.X - MapSize.X / 2.0f) * GridCellSize;
	WorldPos.Y = (GridCoords.Y - MapSize.Y / 2.0f) * GridCellSize;
	WorldPos.Z = 0; // Z определится трейсом
	return WorldPos + GetActorLocation();
}

bool AFogOfWarManager::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (const IMDTeamInterface* ViewerTeam = Cast<IMDTeamInterface>(RealViewer))
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
	// Ячейки 255 (видимо) становятся 127 (туман), 0 (чернота) остается 0.
	for (int32 i = 0; i < RawVisibilityData.Num(); ++i)
	{
		if (RawVisibilityData[i] == 255)
		{
			RawVisibilityData[i] = 127;
		}
	}

	// 3. Обновление обзора от всех живых источников
	// Допустим, у тебя есть список зарегистрированных юнитов
	for (FVisionSource& Source : ActiveVisionSources)
	{
		if (Source.SourceActor && IsValid(Source.SourceActor))
		{
			// Берем позицию юнита и вызываем нашу магию Line of Sight
			UpdateLineOfSight(Source.SourceActor->GetActorLocation(), Source.Radius);
		}
	}

	// 4. Сжатие данных в биты для репликации
	// Это автоматически вызовет OnRep_CompressedFog у клиентов
	for (int32 i = 0; i < RawVisibilityData.Num(); ++i)
	{
		const int32 WordIndex = i / 32;
		const int32 BitIndex = i % 32;

		const bool bIsVisible = (RawVisibilityData[i] == 255);

		if (bIsVisible) CompressedFogData[WordIndex] |= (1 << BitIndex);
		else CompressedFogData[WordIndex] &= ~(1 << BitIndex);
	}

	// ВАЖНО: Принудительно помечаем массив для репликации,
	// так как TArray внутри не всегда триггерит репликацию при изменении элементов
	MARK_PROPERTY_DIRTY_FROM_NAME(AFogOfWarManager, CompressedFogData, this);
}

void AFogOfWarManager::UpdateLineOfSight(FVector Origin, float Radius)
{
	FIntPoint Center = WorldToGrid(Origin);
	const int32 RadiusInCells = FMath::RoundToInt(Radius / GridCellSize);
	const uint8 ViewerHeight = GetTerrainHeight(Center); // Высота смотрящего

	// 1. Определяем границы квадрата для итерации (оптимизация)
	const int32 MinX = FMath::Max(0, Center.X - RadiusInCells);
	const int32 MaxX = FMath::Min(MapSize.X - 1, Center.X + RadiusInCells);
	const int32 MinY = FMath::Max(0, Center.Y - RadiusInCells);
	const int32 MaxY = FMath::Min(MapSize.Y - 1, Center.Y + RadiusInCells);

	// 2. Итерируем по периметру круга и пускаем лучи
	for (int32 x = MinX; x <= MaxX; x++)
	{
		TraceLine(Center, FIntPoint(x, MinY), RadiusInCells, ViewerHeight);
		TraceLine(Center, FIntPoint(x, MaxY), RadiusInCells, ViewerHeight);
	}
	for (int32 y = MinY; y <= MaxY; y++)
	{
		TraceLine(Center, FIntPoint(MinX, y), RadiusInCells, ViewerHeight);
		TraceLine(Center, FIntPoint(MaxX, y), RadiusInCells, ViewerHeight);
	}
}

void AFogOfWarManager::RegisterSource(AActor* InActor, float InRadius)
{
	if (InActor)
	{
		FVisionSource NewSource;
		NewSource.SourceActor = InActor;
		NewSource.Radius = InRadius;
		ActiveVisionSources.Add(NewSource);
	}
}

void AFogOfWarManager::StartFogOfWar()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AFogOfWarManager::CalculateFogOfWar, FogUpdateTick, true);
	}
}

void AFogOfWarManager::EndFogOfWar()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void AFogOfWarManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Реплицируем только владельцу (команде).
	// В Dota сервер знает всё, а клиент — только свой туман.
	DOREPLIFETIME_CONDITION(AFogOfWarManager, CompressedFogData, COND_SkipOwner);
}

void AFogOfWarManager::Tick(float DeltaSeconds)
{
	if (!HasAuthority())
	{
		bool bNeedsTextureUpdate = false;

		for (int32 i = 0; i < CurrentInterpolatedFog.Num(); ++i)
		{
			float OldVal = CurrentInterpolatedFog[i];

			// Плавно движемся к цели
			CurrentInterpolatedFog[i] = FMath::FInterpTo(OldVal, TargetFogGoal[i], DeltaSeconds, FogFadeSpeed);

			// Если значение заметно изменилось, обновляем пиксель
			if (!FMath::IsNearlyEqual(OldVal, CurrentInterpolatedFog[i], 0.01f))
			{
				uint8 Brightness = FMath::RoundToInt(CurrentInterpolatedFog[i] * 255.0f);
				PixelBuffer[i] = FColor(Brightness, Brightness, Brightness, 255);
				bNeedsTextureUpdate = true;
			}
		}

		if (bNeedsTextureUpdate)
		{
			UpdateTexture();
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
		const int32 BitArraySize = FMath::DivideAndRoundUp(TotalCells, 32);
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
	TextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, MapSize.X, MapSize.Y);

	if (PostProcessMaterial)
	{
		// 1. Создаем динамический экземпляр
		FogMaterialInstance = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);

		// 2. Сразу передаем нашу текстуру в параметр "FogMask" (имя из шейдера)
		FogMaterialInstance->SetTextureParameterValue(FName("FogMask"), FogTexture);

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
