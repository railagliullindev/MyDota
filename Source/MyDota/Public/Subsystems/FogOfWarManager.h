// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MyDotaStructTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "FogOfWarManager.generated.h"

/**
 *
 */
UCLASS()
class MYDOTA_API AFogOfWarManager : public AActor
{
	GENERATED_BODY()

public:

	AFogOfWarManager();

	static AFogOfWarManager* Get(const UObject* WorldContextObject);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//-----------------------------------
	// Инициализация
	void InitFogManager();
	void InitTexture();
	//-----------------------------------

	// Настройки сетки
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	float GridCellSize = 100.f; // Размер одной ячейки в юнитах UE

	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	FIntPoint MapSize = FIntPoint(256, 256); // Размер карты в ячейках

	// Метод для обновления видимости от юнита
	void UpdateLineOfSight(FVector Origin, float Radius);

	void RegisterSource(AActor* InActor, float InRadius);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Обновление пикселей (Client Side) */
	UFUNCTION()
	void OnRep_CompressedFog();

	// массив структур для всех источников обзора
	UPROPERTY()
	TArray<FVisionSource> ActiveVisionSources;

private:

	//--------------------------------------------------------------
	// Черновик или рабочая область памяти на сервере
	TArray<uint8> RawVisibilityData;

	// Массив высот и препятствий (Статика)
	TArray<uint8> TerrainHeights;
	TArray<bool> StaticObstacles;

	// Массив для репликации (Биты)
	UPROPERTY(ReplicatedUsing = OnRep_CompressedFog)
	TArray<uint32> CompressedFogData;

	// Массив для текстуры на клиенте (RGBA)
	TArray<FColor> PixelBuffer;
	//--------------------------------------------------------------

	static AFogOfWarManager* Instance;

	void TraceLine(FIntPoint Start, FIntPoint End, int32 MaxRange, uint8 ViewerHeight);

	void UpdateTexture();

	// Вспомогательная функция инициализации
	void BakeLevelData();

	// Хелпер для получения индекса из координат сетки
	FORCEINLINE int32 GetIndex(FIntPoint GridCoords) const
	{
		return GridCoords.X + GridCoords.Y * MapSize.X;
	}

	uint8 GetTerrainHeight(FIntPoint GridCoords) const;
	FVector GridToWorld(FIntPoint GridCoords) const;

	// Чтобы клиент Radiant не получал данные тумана Dire, переопредели проверку релевантности:
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	/** Подготовка текстуры в классе менеджера */
	UPROPERTY()
	UTexture2D* FogTexture;

	// Специальная структура для обновления регионов текстуры
	FUpdateTextureRegion2D* TextureRegion;

	FTimerHandle TimerHandle;

	void CheckAllFogOfWar();

	//-----------------Связка менеджера и материала-------
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	UMaterialInterface* PostProcessMaterialBase;

	UPROPERTY()
	UMaterialInstanceDynamic* FogMaterialInstance;
	//---------------------------------------------------

	//------------------Вычисления-----------------------
	FIntPoint WorldToGrid(FVector Location);
};

/*
 *  if (GetNetMode() < NM_Client) { // Только на сервере
 *  AFogOfWarManager::Get(this)->RegisterSource(this, VisionRadius, TeamID);
}*/