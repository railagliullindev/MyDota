// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/HUD/MinimapWIdget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Controllers/MD_PlayerController.h"
#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "GameFrameworks/MD_GameState.h"
#include "Systems/FogOfWar/FogOfWarManager.h"

void UMinimapWIdget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedFogManager();

	PC = Cast<AMD_PlayerController>(GetOwningPlayer());
	if (!PC) return;

	LocalTeam = PC->GetTeam();

	// Кэшируем GameState
	GS = GetWorld()->GetGameState<AMD_GameState>();

	// Кэшируем FogManager для своей команды
	CachedFogManager();

	// Инициализируем туман войны на миникарте
	if (FogManager && FogManager->GetFogTexture())
	{
		InitMinimapFog(FogManager->GetFogTexture());

		FogManager->OnUnitVisibilityChanged.AddUObject(this, &UMinimapWIdget::OnVisibilityChanged);
	}

	MinimapSize = FVector2D(300, 300);

	// Инициализируем иконки героев с небольшой задержкой,
	// чтобы гарантировать, что все герои уже зарегистрированы в GameState
	GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, this, &UMinimapWIdget::InitHeroIcons, 0.5f, false, 3.f);
}

void UMinimapWIdget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

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

void UMinimapWIdget::NativeDestruct()
{
	if (FogManager)
	{
		FogManager->OnUnitVisibilityChanged.RemoveAll(this);
	}

	ClearHeroIcons();

	Super::NativeDestruct();
}

void UMinimapWIdget::CachedFogManager()
{
	if (!PC) return;

	FogManager = AFogOfWarManager::Get(this, static_cast<uint8>(LocalTeam));

	if (FogManager)
	{
		MapSize = FogManager->GetMapSize();
		GridCellSize = FogManager->GetGridCellSize();
	}
}

void UMinimapWIdget::InitHeroIcons()
{
	if (!GS || !HeroInfoData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMinimapWidget::InitHeroIcons: GS or HeroInfoData is null"));
		return;
	}

	if (!IconHeroClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMinimapWidget::InitHeroIcons: IconHeroClass is null"));
		return;
	}

	ClearHeroIcons();

	// Получаем всех героев своей команды
	const TArray<AActor*>& MyTeamUnits = GS->GetUnitsInTeam(LocalTeam);

	UE_LOG(LogTemp, Log, TEXT("UMinimapWidget::InitHeroIcons: My team has %d units"), MyTeamUnits.Num());

	for (AActor* Unit : MyTeamUnits)
	{
		AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(Unit);
		if (Hero)
		{
			UUserWidget* IconWidget = CreateIconForHero(Hero);
			if (IconWidget)
			{
				HeroIcons.Add(Hero, IconWidget);

				// Свои герои всегда видны на миникарте (если живы)
				IconWidget->SetVisibility(ESlateVisibility::Visible);

				UE_LOG(LogTemp, Log, TEXT("Created icon for friendly hero: %s"), *Hero->GetName());
			}
		}
	}

	// Получаем всех героев вражеской команды
	EMDTeam EnemyTeam = AFogOfWarManager::GetEnemyTeam(LocalTeam);
	const TArray<AActor*>& EnemyTeamUnits = GS->GetUnitsInTeam(EnemyTeam);

	for (AActor* Unit : EnemyTeamUnits)
	{
		AMD_CharacterBase* Hero = Cast<AMD_CharacterBase>(Unit);
		if (Hero)
		{
			UUserWidget* IconWidget = CreateIconForHero(Hero);
			if (IconWidget)
			{
				HeroIcons.Add(Hero, IconWidget);

				bool bInitiallyVisible = true;
				IconWidget->SetVisibility(bInitiallyVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

				UE_LOG(LogTemp, Log, TEXT("Created icon for enemy hero: %s, initially %s"), *Hero->GetName(), bInitiallyVisible ? TEXT("Visible") : TEXT("Hidden"));
			}
		}
	}
}

void UMinimapWIdget::UpdateHeroIcon(AActor* InActor, UUserWidget* IconWidget)
{
	if (!IsValid(InActor) || !IsValid(IconWidget)) return;

	// Проверяем, жив ли герой
	bool bIsHeroDead = DeadHeroes.Contains(InActor);

	// Получаем команду героя
	EMDTeam HeroTeam = EMDTeam::None;
	AMD_CharacterBase* HeroCharacter = Cast<AMD_CharacterBase>(InActor);
	if (HeroCharacter)
	{
		HeroTeam = HeroCharacter->GetTeam();
	}

	// Определяем должна ли иконка быть видимой
	bool bShouldBeVisible = false;

	if (HeroTeam == LocalTeam)
	{
		// Свои герои всегда видны, если живы
		bShouldBeVisible = !bIsHeroDead;
	}
	else
	{
		// Для вражеских героев видимость управляется через OnUnitVisibilityChanged
		// Здесь мы просто проверяем текущее состояние видимости иконки
		// НЕ используем FogManager->IsCellVisibleOnClient!
		bShouldBeVisible = (IconWidget->GetVisibility() == ESlateVisibility::Visible) && !bIsHeroDead;
	}

	// Если герой мертв, скрываем иконку независимо от других условий
	if (bIsHeroDead)
	{
		bShouldBeVisible = false;
	}

	// Устанавливаем видимость (избегаем лишних вызовов)
	ESlateVisibility CurrentVisibility = IconWidget->GetVisibility();
	ESlateVisibility NewVisibility = bShouldBeVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (CurrentVisibility != NewVisibility)
	{
		IconWidget->SetVisibility(NewVisibility);
	}

	// Обновляем позицию только если иконка видима
	if (bShouldBeVisible)
	{
		FVector2D NormPos = GetNormalizedPosition(InActor->GetActorLocation());

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
		{
			// FVector2D CanvasSize = MinimapCanvas->GetTickSpaceGeometry().GetLocalSize();
			FVector2D FinalPos = FVector2D(NormPos.X * MinimapSize.X, NormPos.Y * MinimapSize.Y);

			// Избегаем лишних обновлений позиции
			FVector2D CurrentPos = CanvasSlot->GetPosition();
			if (!CurrentPos.Equals(FinalPos, 0.1f))
			{
				CanvasSlot->SetPosition(FinalPos);
			}
		}

		/*// 1. Получаем нормализованные координаты (0..1)
		FVector2D NormPos = GetNormalizedPosition(InActor->GetActorLocation());

		// 2. Получаем размер контейнера (Canvas), в котором лежат иконки
		// Важно: берем размер именно того CanvasPanel, который является "картой"
		FVector2D MapSizeOnScreen = MinimapSize; // MinimapCanvas->GetTickSpaceGeometry().GetLocalSize();

		// 3. ПРАВИЛЬНОЕ СОПОСТАВЛЕНИЕ:
		// Твое смещение в пикселях:
		FVector2D FinalPos;

		// X виджета (горизонталь) = NormPos.X * Ширина
		FinalPos.X = NormPos.X * MapSizeOnScreen.X;

		// Y виджета (вертикаль) = NormPos.Y * Высота
		FinalPos.Y = NormPos.Y * MapSizeOnScreen.Y;

		// 4. Устанавливаем позицию
		// Если иконка — это Child в Canvas Panel:
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
		{
			CanvasSlot->SetPosition(FinalPos);
		}*/
	}
}

FVector2D UMinimapWIdget::GetNormalizedPosition(const FVector& WorldLocation)
{
	// 1. Смещение (относительно центра карты 0,0)
	FVector RelativePos = WorldLocation;
	float FullMapSize = MapSize.X * GridCellSize;

	// 2. Свап осей
	float RawNormX = (RelativePos.Y / FullMapSize) + 0.5f;
	float RawNormY = (RelativePos.X / FullMapSize) + 0.5f;

	// 3. ИНВЕРСИЯ ВЕРТИКАЛИ:
	// В мире X+ это ВПЕРЕД (вверх карты).
	// В UI 0.0 это ВЕРХ. Поэтому 0.5 + AlphaX нужно превратить в 0.5 - AlphaX
	// Формула: 1.0 - RawNormY
	float FinalUI_X = FMath::Clamp(RawNormX, 0.0f, 1.0f);
	float FinalUI_Y = FMath::Clamp(1.0f - RawNormY, 0.0f, 1.0f);

	return FVector2D(FinalUI_X, FinalUI_Y);
}

void UMinimapWIdget::OnVisibilityChanged(AActor* InActor, bool IsVisible)
{
	if (!InActor) return;

	// Нас интересуют только вражеские юниты
	AMD_CharacterBase* Character = Cast<AMD_CharacterBase>(InActor);
	if (!Character || Character->GetTeam() == LocalTeam) return;

	// Обновляем видимость иконки для этого юнита
	if (HeroIcons.Contains(InActor))
	{
		UUserWidget* IconWidget = HeroIcons[InActor];
		if (IsValid(IconWidget))
		{
			// Проверяем, не мертв ли герой
			bool bIsDead = DeadHeroes.Contains(InActor);

			// Устанавливаем видимость на основе данных из FogManager
			ESlateVisibility NewVisibility = (IsVisible && !bIsDead) ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

			if (IconWidget->GetVisibility() != NewVisibility)
			{
				IconWidget->SetVisibility(NewVisibility);

				UE_LOG(LogTemp, Verbose, TEXT("Hero %s visibility changed to %s"), *InActor->GetName(), IsVisible ? TEXT("Visible") : TEXT("Hidden"));
			}
		}
	}
}

void UMinimapWIdget::OnHeroDied(AActor* InActor)
{
	if (InActor && HeroIcons.Contains(InActor))
	{
		DeadHeroes.Add(InActor);

		// Скрываем иконку
		UUserWidget* IconWidget = HeroIcons[InActor];
		if (IsValid(IconWidget))
		{
			IconWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UMinimapWIdget::ClearHeroIcons()
{
	for (auto& Pair : HeroIcons)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->RemoveFromParent();
		}
	}
	HeroIcons.Empty();
	DeadHeroes.Empty();
}

void UMinimapWIdget::InitMinimapFog(UTexture2D* InFogTexture)
{
	if (!MinimapMaterial || !InFogTexture || !MinimapImage) return;

	MinimapMID = UMaterialInstanceDynamic::Create(MinimapMaterial, this);
	if (MinimapMID)
	{
		MinimapMID->SetTextureParameterValue(FName("FogMask"), InFogTexture);
		MinimapImage->SetBrushFromMaterial(MinimapMID);
	}
}

UUserWidget* UMinimapWIdget::CreateIconForHero(AMD_CharacterBase* InHero)
{
	if (!InHero || !IconHeroClass || !MinimapCanvas) return nullptr;

	// Создаем виджет иконки
	UUserWidget* IconWidget = CreateWidget<UUserWidget>(this, IconHeroClass);
	if (!IconWidget) return nullptr;

	// Добавляем на Canvas
	UCanvasPanelSlot* CanvasSlot = MinimapCanvas->AddChildToCanvas(IconWidget);
	if (CanvasSlot)
	{
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetAutoSize(true);
	}

	// Установка иконки
	if (HeroInfoData)
	{
		UTexture2D* HeroIconTexture = HeroInfoData->GetIconForHero(InHero);
		if (HeroIconTexture)
		{
			// Предполагаем, что в IconHeroClass есть Image с именем "HeroIcon"
			UImage* HeroIconImage = Cast<UImage>(IconWidget->GetWidgetFromName(TEXT("HeroIcon")));
			if (HeroIconImage)
			{
				HeroIconImage->SetBrushFromTexture(HeroIconTexture);
			}
		}
	}

	return IconWidget;
}