// Rail Agliullin Dev. All Rights Reserved


#include "Subsystems/FogOfWarManager.h"

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

AFogOfWarManager::AFogOfWarManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AFogOfWarManager::BeginPlay()
{
	Super::BeginPlay();
	Instance = this; // Регистрируем себя при спавне
	
	InitFogManager();
	
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AFogOfWarManager::CheckAllFogOfWar, 0.1f, true);
	}
}

void AFogOfWarManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Instance == this) Instance = nullptr;
	Super::EndPlay(EndPlayReason);
}

void AFogOfWarManager::OnRep_CompressedFog()
{
	PixelBuffer[0] = FColor::Red;
	
	// Проверка
	// for(auto& Color : PixelBuffer) Color = FColor::Red;
	
	for (int32 i = 0; i < PixelBuffer.Num(); ++i)
	{
		int32 WordIndex = i / 32;
		int32 BitIndex = i % 32;
		bool bIsVisible = (CompressedFogData[WordIndex] >> BitIndex) & 1;

		// Текущее состояние в буфере
		FColor TargetColor = bIsVisible ? FColor::White : FColor(127, 127, 127, 255);
        
		// Для плавности можно делать Lerp между старым цветом и новым, 
		// но для начала просто записываем:
		PixelBuffer[i] = TargetColor;
	}

	UpdateTexture();
}

AFogOfWarManager* AFogOfWarManager::Instance = nullptr;

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

		if (error > 0) {
			x += x_inc;
			error -= dy;
		} else {
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
	auto RegionData = TextureRegion;
	auto DataPtr = PixelBuffer.GetData();
    
	FogTexture->UpdateTextureRegions(0, 1, RegionData, MapSize.X * 4, 4, (uint8*)DataPtr);
}

FIntPoint AFogOfWarManager::WorldToGrid(FVector Location)
{
	// 1. Смещение от центра (0,0,0)
	// 2. Делим на размер ячейки (100)
	// 3. Прибавляем половину сетки (29), чтобы (0,0) мира стал (29,29) в сетке
	int32 GridX = FMath::FloorToInt(Location.X / GridCellSize) + (MapSize.X / 2);
	int32 GridY = FMath::FloorToInt(Location.Y / GridCellSize) + (MapSize.Y / 2);

	// Обязательный зажим в границы массива [0...57]
	GridX = FMath::Clamp(GridX, 0, MapSize.X - 1);
	GridY = FMath::Clamp(GridY, 0, MapSize.Y - 1);

	return FIntPoint(GridX, GridY);
}

void AFogOfWarManager::BakeLevelData()
{
	int32 TotalCells = MapSize.X * MapSize.Y;
	
	if (TotalCells <= 0) {
		UE_LOG(LogTemp, Error, TEXT("MapSize is Zero! Check your Blueprint/Editor settings."));
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
			FVector RayEnd = CellWorldPos - FVector(0, 0, 2000.f);   // В пол

			FHitResult Hit;
			// Трейсим, чтобы найти землю (Landscape) и объекты (Trees/Walls)
			if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_WorldStatic, QueryParams))
			{
				// 1. Записываем высоту (делим на шаг высоты Dota, например, 128 юнитов)
				// Это превратит 0, 128, 256 в уровни 0, 1, 2
				TerrainHeights[Index] = FMath::Clamp(FMath::FloorToInt(Hit.Location.Z / 128.f), 0, 255);

				// 2. Проверяем, является ли объект препятствием
				// Можно проверять по Tag или по Actor Class (например, деревья)
				if (Hit.GetActor() && Hit.GetActor()->ActorHasTag("BlockFog")) {
					StaticObstacles[Index] = true;
				} else {
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

FVector AFogOfWarManager::GridToWorld(FIntPoint GridCoords) const
{
	FVector WorldPos;
	WorldPos.X = (GridCoords.X - MapSize.X / 2.0f) * GridCellSize;
	WorldPos.Y = (GridCoords.Y - MapSize.Y / 2.0f) * GridCellSize;
	WorldPos.Z = 0; // Z определится трейсом
	return WorldPos + GetActorLocation();
}

bool AFogOfWarManager::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	// Получаем команду игрока, который запрашивает данные
	//IMyTeamInterface* ViewerTeam = Cast<IMyTeamInterface>(RealViewer);
	//return ViewerTeam && ViewerTeam->GetTeam() == this->AssignedTeam;
	
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
	
	//return false;
}



void AFogOfWarManager::CheckAllFogOfWar()
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

	
	FString Who = HasAuthority() ? "Server" : "Client"; 
	UE_LOG(LogTemp, Warning, TEXT("Fog [%s] : Sources Count = %d"), *Who, ActiveVisionSources.Num());
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
		int32 WordIndex = i / 32;
		int32 BitIndex = i % 32;
        
		bool bIsVisible = (RawVisibilityData[i] == 255);
        
		if (bIsVisible) 
			CompressedFogData[WordIndex] |= (1 << BitIndex);
		else 
			CompressedFogData[WordIndex] &= ~(1 << BitIndex);
	}

	// ВАЖНО: Принудительно помечаем массив для репликации, 
	// так как TArray внутри не всегда триггерит репликацию при изменении элементов
	MARK_PROPERTY_DIRTY_FROM_NAME(AFogOfWarManager, CompressedFogData, this);
}

void AFogOfWarManager::UpdateLineOfSight(FVector Origin, float Radius)
{
	FIntPoint Center = WorldToGrid(Origin);
	int32 RadiusInCells = FMath::RoundToInt(Radius / GridCellSize);
	uint8 ViewerHeight = GetTerrainHeight(Center); // Высота смотрящего

	// 1. Определяем границы квадрата для итерации (оптимизация)
	int32 MinX = FMath::Max(0, Center.X - RadiusInCells);
	int32 MaxX = FMath::Min(MapSize.X - 1, Center.X + RadiusInCells);
	int32 MinY = FMath::Max(0, Center.Y - RadiusInCells);
	int32 MaxY = FMath::Min(MapSize.Y - 1, Center.Y + RadiusInCells);

	// 2. Итерируем по периметру круга и пускаем лучи
	for (int32 x = MinX; x <= MaxX; x++) {
		TraceLine(Center, FIntPoint(x, MinY), RadiusInCells, ViewerHeight);
		TraceLine(Center, FIntPoint(x, MaxY), RadiusInCells, ViewerHeight);
	}
	for (int32 y = MinY; y <= MaxY; y++) {
		TraceLine(Center, FIntPoint(MinX, y), RadiusInCells, ViewerHeight);
		TraceLine(Center, FIntPoint(MaxX, y), RadiusInCells, ViewerHeight);
	}
}

void AFogOfWarManager::RegisterSource(AActor* InActor, float InRadius)
{	
	UE_LOG(LogTemp, Warning, TEXT("RegisterSource"));
	
	if (InActor)
	{
		FVisionSource NewSource;
		NewSource.SourceActor = InActor;
		NewSource.Radius = InRadius;
		ActiveVisionSources.Add(NewSource);
		
		UE_LOG(LogTemp, Log, TEXT("FogManager: New source added! Total sources: %d"), ActiveVisionSources.Num());
	}
}

AFogOfWarManager* AFogOfWarManager::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;
	
	for (TActorIterator<AFogOfWarManager> It(World); It; ++It)
	{
		return *It; // Возвращаем первого найденного актора на этой сцене
	}
	
	return nullptr;
}

void AFogOfWarManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Реплицируем только владельцу (команде). 
	// В Dota сервер знает всё, а клиент — только свой туман.
	DOREPLIFETIME_CONDITION(AFogOfWarManager, CompressedFogData, COND_SkipOwner);
}

void AFogOfWarManager::InitFogManager()
{
	// 1. Защита от нулевого размера
	if (MapSize.X <= 0 || MapSize.Y <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("FogManager: MapSize is Invalid (%d, %d)!"), MapSize.X, MapSize.Y);
		return;
	}

	int32 TotalCells = MapSize.X * MapSize.Y;
	UE_LOG(LogTemp, Warning, TEXT("FogManager: Initializing for %d cells..."), TotalCells);

	// 2. Инициализация статических данных (Нужны и серверу, и клиенту для рейкастов/логики)
	TerrainHeights.Empty();
	TerrainHeights.SetNumZeroed(TotalCells);

	StaticObstacles.Empty();
	StaticObstacles.SetNumZeroed(TotalCells);

	// 3. Инициализация серверных данных
	if (HasAuthority()) // Только на сервере
	{
		RawVisibilityData.Empty();
		RawVisibilityData.SetNumZeroed(TotalCells);

		// Размер битового массива: количество ячеек / 32 (округляем вверх)
		int32 BitArraySize = FMath::DivideAndRoundUp(TotalCells, 32);
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

	UE_LOG(LogTemp, Log, TEXT("FogManager: Initialization Complete."));
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
	
	
	if (PostProcessMaterialBase)
	{
		// 1. Создаем динамический экземпляр
		FogMaterialInstance = UMaterialInstanceDynamic::Create(PostProcessMaterialBase, this);
    
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


