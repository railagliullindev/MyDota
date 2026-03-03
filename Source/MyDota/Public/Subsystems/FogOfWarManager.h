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

	static AFogOfWarManager* Get(const UObject* WorldContextObject, uint8 TeamID = -1);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void InitFogManager();

	UPROPERTY(BlueprintReadOnly, Category = "Fog | Teams")
	EMDTeam AssignedTeamID;

	/** Размер одной ячейки в юнитах UE */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	float GridCellSize = 100.f;

	/** Размер карты в ячейках */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	FIntPoint MapSize = FIntPoint(256, 256);

	/** Метод для обновления видимости от юнита */
	void UpdateLineOfSight(FVector Origin, float Radius);

	/** Регистрация в сети */
	void RegisterSource(AActor* InActor, float InRadius);

	void StartFogOfWar();
	void EndFogOfWar();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Обновление пикселей (Client Side) */
	UFUNCTION()
	void OnRep_CompressedFog();

	/** Массив Actor'ов для расчета видимости */
	UPROPERTY()
	TArray<FVisionSource> ActiveVisionSources;

private:

	/** Чтобы клиент Radiant не получал данные тумана Dire, переопредели проверку релевантности */
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	/** Материал тумана войны (Post Process) */
	UPROPERTY(EditDefaultsOnly, Category = "Fog Settings")
	UMaterialInterface* PostProcessMaterial;

	/** Динамический экземпляр материала */
	UPROPERTY()
	UMaterialInstanceDynamic* FogMaterialInstance;

	/** Черновик или рабочая область памяти на сервере */
	TArray<uint8> RawVisibilityData;

	/** Массив высот */
	TArray<uint8> TerrainHeights;

	/** Массив статических препятсвий */
	TArray<bool> StaticObstacles;

	/** Массив для репликации (Биты) */
	UPROPERTY(ReplicatedUsing = OnRep_CompressedFog)
	TArray<uint32> CompressedFogData;

	/** Массив для текстуры на клиенте (RGBA) */
	TArray<FColor> PixelBuffer;

	/** Подготовка текстуры в классе менеджера */
	UPROPERTY()
	UTexture2D* FogTexture;

	/** Специальная структура для обновления регионов текстуры */
	FUpdateTextureRegion2D* TextureRegion;

	/** Хелпер для получения индекса из координат сетки */
	FORCEINLINE int32 GetIndex(FIntPoint GridCoords) const
	{
		return GridCoords.X + GridCoords.Y * MapSize.X;
	}

	// Вспомогательная функция инициализации
	void BakeLevelData();

	/** Инициализация текстуры маски для шейдера */
	void InitTexture();

	/** Передача данных из PixelBuffer в нашу текстуру */
	void UpdateTexture();

	/** Рассчитать туман войны */
	void CalculateFogOfWar();

	FTimerHandle TimerHandle;

	/** Получить ячейку по из мировых координат */
	FIntPoint WorldToGrid(FVector Location) const;

	/** Получить мировые координаты из ячейки */
	FVector GridToWorld(FIntPoint GridCoords) const;

	/** Расчет видимости */
	void TraceLine(FIntPoint Start, FIntPoint End, int32 MaxRange, uint8 ViewerHeight);

	/** Получить высоту ячейки */
	uint8 GetTerrainHeight(FIntPoint GridCoords) const;

	bool bInitialized = false;
};