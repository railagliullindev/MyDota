// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MyDotaStructTypes.h"
#include "Blueprint/UserWidget.h"
#include "MinimapWidget.generated.h"

class AMD_PlayerController;
class UCanvasPanel;
enum class EMDTeam : uint8;
class AMD_GameState;
class UImage;
class UMDHeroInfoDataAsset;
class AFogOfWarManager;

/**
 *
 */
UCLASS()
class MYDOTA_API UMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

	/* ============================================================================
	 *  Публичный интерфейс
	 * ============================================================================ */
public:

	// --- Переопределения -------------------------------------------------------
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	/* ============================================================================
	 *  Свойства (Настройки в редакторе)
	 * ============================================================================ */
protected:

	/** Датаассет с информацией о героях (иконки и т.д.) */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	UMDHeroInfoDataAsset* HeroInfoData;

	/** Класс виджета иконки героя */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<UUserWidget> IconHeroClass;

	/** Материал для миникарты (с туманом войны) */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	UMaterialInterface* MinimapMaterial;

	/** Размер миникарты в виджете (пиксели) */
	UPROPERTY(EditDefaultsOnly, Category = "Setup", meta = (ClampMin = "50"))
	FVector2D WidgetSize = FVector2D(300.f, 300.f);

	/** Название картинки в IconHeroClass, в которую нужно засунуть иконку */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FName IconHeroImageName = FName("HeroIcon");

	/** Название параметра текстуры InFogTexture, для наложения тумана */
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FName FogMaskParamName = FName("FogMask");

	/* ============================================================================
	 *  BindWidget свойства
	 * ============================================================================ */
protected:

	/** Изображение миникарты (фон) */
	UPROPERTY(meta = (BindWidget))
	UImage* MinimapImage;

	/** Canvas для размещения иконок героев */
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MinimapCanvas;

	/* ============================================================================
	 *  Внутренние данные
	 * ============================================================================ */
private:

	// --- Кэшированные указатели ------------------------------------------------
	UPROPERTY()
	AFogOfWarManager* FogManager = nullptr;

	UPROPERTY()
	AMD_GameState* GS = nullptr;

	UPROPERTY()
	AMD_PlayerController* PC = nullptr;

	// --- Данные миникарты ------------------------------------------------------
	/** Размер карты в ячейках (из FogManager) */
	FIntPoint MapSize;

	/** Размер ячейки в юнитах (из FogManager) */
	float GridCellSize = 100.f;

	/** Размер карты в юнитах (кэш) */
	float WorldMapSize = 0.f;

	// --- Состояние -------------------------------------------------------------
	/** Команда локального игрока */
	EMDTeam LocalTeam = EMDTeam::None;

	/** Карта иконок героев */
	UPROPERTY()
	TMap<AActor*, UUserWidget*> HeroIcons;

	/** Множество мертвых героев */
	UPROPERTY()
	TSet<AActor*> DeadHeroes;

	/** Динамический материал для миникарты */
	UPROPERTY()
	UMaterialInstanceDynamic* MinimapMID = nullptr;

	/** Таймер для инициализации */
	FTimerHandle InitTimerHandle;

	/** Флаг инициализации */
	bool bInitialized = false;

	/* ============================================================================
	 *  Приватные методы
	 * ============================================================================ */
private:

	// --- Инициализация ---------------------------------------------------------
	/** Инициализировать туман войны на миникарте */
	void InitMinimapFog(UTexture2D* InFogTexture);

	/** Инициализировать иконки героев */
	void InitHeroIcons();

	/** Создать иконку для героя */
	UUserWidget* CreateIconForHero(AActor* InActor);

	// --- Обновление ------------------------------------------------------------
	/** Обновить позицию иконки героя */
	void UpdateHeroIcon(AActor* InActor, UUserWidget* IconWidget);

	/** Получить нормализованную позицию (0-1) для мировой координаты */
	FVector2D GetNormalizedPosition(const FVector& WorldLocation) const;

	// --- Обработчики событий ---------------------------------------------------
	/** Обработчик изменения видимости юнита (из FogManager) */
	UFUNCTION()
	void OnVisibilityChanged(AActor* InActor, bool bIsVisible);

	/** Обработчик смерти героя */
	UFUNCTION()
	void OnHeroDied(AActor* InActor);

	// --- Очистка ---------------------------------------------------------------
	/** Очистить все иконки героев */
	void ClearHeroIcons();

	/** Обновить кэшированные данные из FogManager */
	void UpdateCachedData();

	/** Обновить видимость конкретного героя */
	void UpdateHeroVisibility(AActor* InActor);

	/** Подписаться на события героя */
	void SubscribeToHeroEvents(AActor* InActor);

	/** Отписаться от событий героя */
	void UnsubscribeFromHeroEvents(AActor* InActor);
};
