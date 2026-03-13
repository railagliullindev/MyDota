// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/MinimapWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Controllers/MD_PlayerController.h"
#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "GameFrameworks/MD_GameState.h"
#include "Subsystems/MD_DataSubsystem.h"
#include "Systems/FogOfWar/FogOfWarManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMinimap, Log, All);

void UMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Кэшируем указатели
	PC = Cast<AMD_PlayerController>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogMinimap, Error, TEXT("Failed to get PlayerController"));
		return;
	}

	GS = GetWorld()->GetGameState<AMD_GameState>();
	if (!GS)
	{
		UE_LOG(LogMinimap, Error, TEXT("Failed to get GameState"));
		return;
	}

	LocalTeam = PC->GetTeam();
	UpdateCachedData();

	// Инициализируем туман войны
	if (FogManager && FogManager->GetFogTexture())
	{
		InitMinimapFog(FogManager->GetFogTexture());
		FogManager->OnUnitVisibilityChanged.AddUObject(this, &UMinimapWidget::OnVisibilityChanged);
	}
	else
	{
		UE_LOG(LogMinimap, Warning, TEXT("FogManager or FogTexture is null"));
	}

	// Инициализируем иконки с задержкой
	GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &UMinimapWidget::InitHeroIcons, 0.1f, false, 3.f);
}

void UMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Обновляем позиции всех иконок
	for (auto& Pair : HeroIcons)
	{
		AActor* Hero = Pair.Key;
		UUserWidget* IconWidget = Pair.Value;

		if (!IsValid(Hero) || !IsValid(IconWidget))
		{
			continue;
		}

		UpdateHeroIcon(Hero, IconWidget);
	}
}

void UMinimapWidget::NativeDestruct()
{
	if (FogManager)
	{
		FogManager->OnUnitVisibilityChanged.RemoveAll(this);
	}

	ClearHeroIcons();

	Super::NativeDestruct();
}

void UMinimapWidget::InitHeroIcons()
{
	if (!GS || !IconHeroClass)
	{
		UE_LOG(LogMinimap, Warning, TEXT("Cannot init hero icons: missing dependencies"));
		return;
	}

	// Очищаем старые иконки
	ClearHeroIcons();

	// Получаем вражескую команду
	const EMDTeam EnemyTeam = AFogOfWarManager::GetEnemyTeam(LocalTeam);

	// Функция для создания иконок для команды
	auto CreateIconsForTeam = [this](EMDTeam Team, bool bIsFriendly)
	{
		const TArray<AActor*>& TeamUnits = GS->GetUnitsInTeam(Team);

		for (AActor* Unit : TeamUnits)
		{
			AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(Unit);
			if (!Hero) continue;

			UUserWidget* IconWidget = CreateIconForHero(Hero);
			if (!IconWidget) continue;

			HeroIcons.Add(Hero, IconWidget);

			// Подписываемся на события героя
			SubscribeToHeroEvents(Hero);

			// Устанавливаем начальную видимость
			UpdateHeroVisibility(Hero);

			UE_LOG(LogMinimap, Log, TEXT("Created icon for %s hero: %s"), bIsFriendly ? TEXT("friendly") : TEXT("enemy"), *Hero->GetName());
		}
	};

	CreateIconsForTeam(LocalTeam, true);
	CreateIconsForTeam(EnemyTeam, false);

	bInitialized = true;
	UE_LOG(LogMinimap, Verbose, TEXT("Hero icons initialized: %d total"), HeroIcons.Num());
}

void UMinimapWidget::UpdateHeroIcon(AActor* InActor, UUserWidget* IconWidget)
{
	if (!IsValid(InActor) || !IsValid(IconWidget)) return;

	// Проверяем, жив ли герой
	const bool bIsDead = DeadHeroes.Contains(InActor);

	// Определяем команду героя
	EMDTeam HeroTeam = EMDTeam::None;
	if (const AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(InActor))
	{
		HeroTeam = Hero->GetTeam();
	}

	// Определяем должна ли иконка быть видимой
	bool bShouldBeVisible = false;

	if (HeroTeam == LocalTeam)
	{
		// Свои герои всегда видны, если живы
		bShouldBeVisible = !bIsDead;
	}
	else
	{
		// Для врагов видимость определяется FogManager
		// Иконка уже скрыта/показана через OnVisibilityChanged
		bShouldBeVisible = (IconWidget->GetVisibility() == ESlateVisibility::Visible) && !bIsDead;
	}

	// Обновляем видимость (только если изменилась)
	const ESlateVisibility NewVisibility = bShouldBeVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (IconWidget->GetVisibility() != NewVisibility)
	{
		IconWidget->SetVisibility(NewVisibility);
	}

	// Обновляем позицию только для видимых иконок
	if (bShouldBeVisible)
	{
		const FVector2D NormPos = GetNormalizedPosition(InActor->GetActorLocation());
		const FVector2D FinalPos = FVector2D(NormPos.X * WidgetSize.X, NormPos.Y * WidgetSize.Y);

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
		{
			const FVector2D CurrentPos = CanvasSlot->GetPosition();

			// Обновляем только если позиция изменилась (с допуском 0.5 пикселя)
			if (!CurrentPos.Equals(FinalPos, 0.5f))
			{
				CanvasSlot->SetPosition(FinalPos);
			}
		}
	}
}

FVector2D UMinimapWidget::GetNormalizedPosition(const FVector& WorldLocation) const
{
	if (WorldMapSize <= 0)
	{
		return FVector2D::ZeroVector;
	}

	// Преобразуем мировые координаты в нормализованные (0-1)
	// X мира -> Y на миникарте, Y мира -> X на миникарте
	const float RawX = (WorldLocation.Y / WorldMapSize) + 0.5f;
	const float RawY = (WorldLocation.X / WorldMapSize) + 0.5f;

	// Инвертируем Y (в мире вперед = вниз на миникарте)
	return FVector2D(FMath::Clamp(RawX, 0.0f, 1.0f), FMath::Clamp(1.0f - RawY, 0.0f, 1.0f));
}

void UMinimapWidget::OnVisibilityChanged(AActor* InActor, bool bIsVisible)
{
	if (!IsValid(InActor)) return;

	// TODO: Проверить появляются ли тут свои? (не должно!)

	// Обновляем видимость иконки врага
	if (UUserWidget* const* IconPtr = HeroIcons.Find(InActor))
	{
		UUserWidget* IconWidget = *IconPtr;
		if (IsValid(IconWidget))
		{
			const bool bIsDead = DeadHeroes.Contains(InActor);
			const ESlateVisibility NewVisibility = (bIsVisible && !bIsDead) ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

			if (IconWidget->GetVisibility() != NewVisibility)
			{
				IconWidget->SetVisibility(NewVisibility);

				UE_LOG(LogMinimap, VeryVerbose, TEXT("Enemy %s visibility: %s"), *InActor->GetName(), bIsVisible ? TEXT("Visible") : TEXT("Hidden"));
			}
		}
	}
}

void UMinimapWidget::OnHeroDied(AActor* InActor)
{
	if (!IsValid(InActor) || !HeroIcons.Contains(InActor)) return;

	DeadHeroes.Add(InActor);

	// Скрываем иконку
	if (UUserWidget* IconWidget = HeroIcons[InActor])
	{
		IconWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	UnsubscribeFromHeroEvents(InActor);

	UE_LOG(LogMinimap, Verbose, TEXT("Hero died: %s"), *InActor->GetName());
}

void UMinimapWidget::ClearHeroIcons()
{
	for (const auto& Pair : HeroIcons)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->RemoveFromParent();
		}

		// Отписываемся от смерти
		if (IsValid(Pair.Key))
		{
			Pair.Key->OnDestroyed.RemoveDynamic(this, &UMinimapWidget::OnHeroDied);
		}
	}

	HeroIcons.Empty();
	DeadHeroes.Empty();

	UE_LOG(LogMinimap, Verbose, TEXT("Hero icons cleared"));
}

void UMinimapWidget::UpdateCachedData()
{
	if (!PC) return;

	FogManager = AFogOfWarManager::Get(this, static_cast<uint8>(LocalTeam));

	if (FogManager)
	{
		MapSize = FogManager->GetMapSize();
		GridCellSize = FogManager->GetGridCellSize();
		WorldMapSize = MapSize.X * GridCellSize;

		UE_LOG(LogMinimap, Verbose, TEXT("Cached data updated: MapSize=%d,%d, GridCellSize=%.1f"), MapSize.X, MapSize.Y, GridCellSize);
	}
	else
	{
		UE_LOG(LogMinimap, Warning, TEXT("Failed to get FogManager for team %d"), LocalTeam);
	}
}

void UMinimapWidget::UpdateHeroVisibility(AActor* InActor)
{
	if (!IsValid(InActor) || !FogManager) return;

	UUserWidget* const* IconPtr = HeroIcons.Find(InActor);
	if (!IconPtr) return;

	UUserWidget* IconWidget = *IconPtr;
	if (!IsValid(IconWidget)) return;

	const AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(InActor);
	if (!Hero) return;

	const bool bIsDead = DeadHeroes.Contains(InActor);
	const bool bIsFriendly = (Hero->GetTeam() == LocalTeam);

	// Определяем должна ли иконка быть видимой
	bool bShouldBeVisible = false;

	if (bIsFriendly)
	{
		// Свои герои всегда видны, если живы
		bShouldBeVisible = !bIsDead;
	}
	else
	{
		// Для врагов - проверяем видимость в тумане войны
		const FIntPoint GridPos = FogManager->WorldToGrid(InActor->GetActorLocation());
		const bool bIsVisibleInFog = FogManager->IsCellVisibleOnClient(GridPos);
		bShouldBeVisible = bIsVisibleInFog && !bIsDead;
	}

	// Обновляем видимость
	const ESlateVisibility NewVisibility = bShouldBeVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (IconWidget->GetVisibility() != NewVisibility)
	{
		IconWidget->SetVisibility(NewVisibility);

		UE_LOG(LogMinimap, Verbose, TEXT("Hero %s visibility updated to %s"), *InActor->GetName(), bShouldBeVisible ? TEXT("Visible") : TEXT("Hidden"));
	}
}

void UMinimapWidget::SubscribeToHeroEvents(AActor* InActor)
{
	if (!InActor) return;

	// Подписываемся на смерть
	if (!InActor->OnDestroyed.IsAlreadyBound(this, &UMinimapWidget::OnHeroDied))
	{
		InActor->OnDestroyed.AddDynamic(this, &UMinimapWidget::OnHeroDied);
	}
}

void UMinimapWidget::UnsubscribeFromHeroEvents(AActor* InActor)
{
	if (!IsValid(InActor)) return;

	InActor->OnDestroyed.RemoveDynamic(this, &UMinimapWidget::OnHeroDied);
}

void UMinimapWidget::InitMinimapFog(UTexture2D* InFogTexture)
{
	if (!MinimapMaterial || !InFogTexture || !MinimapImage) return;

	MinimapMID = UMaterialInstanceDynamic::Create(MinimapMaterial, this);
	if (MinimapMID)
	{
		MinimapMID->SetTextureParameterValue(FogMaskParamName, InFogTexture);
		MinimapImage->SetBrushFromMaterial(MinimapMID);

		UE_LOG(LogMinimap, Verbose, TEXT("Minimap fog initialized"));
	}
}

UUserWidget* UMinimapWidget::CreateIconForHero(AActor* InActor)
{
	if (!InActor || !IconHeroClass || !MinimapCanvas) return nullptr;

	// Создаем виджет
	UUserWidget* IconWidget = CreateWidget<UUserWidget>(this, IconHeroClass);
	if (!IconWidget) return nullptr;

	// Добавляем на Canvas
	UCanvasPanelSlot* CanvasSlot = MinimapCanvas->AddChildToCanvas(IconWidget);
	if (CanvasSlot)
	{
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetAutoSize(true);
	}

	auto HeroInfo = UMD_DataSubsystem::Get(this)->GetHeroInfo(InActor);
	// Устанавливаем иконку героя
	if (HeroInfo.IsValid())
	{
		if (UTexture2D* HeroIconTexture = HeroInfo.HeroMinimapIcon)
		{
			if (UImage* HeroIconImage = Cast<UImage>(IconWidget->GetWidgetFromName(IconHeroImageName)))
			{
				HeroIconImage->SetBrushFromTexture(HeroIconTexture);
				UE_LOG(LogMinimap, Verbose, TEXT("Minimap fog initialized"));
			}
		}
	}

	return IconWidget;
}