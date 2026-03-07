// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/HUD/MinimapWIdget.h"

#include "Components/Image.h"
#include "Controllers/MD_PlayerController.h"
#include "GameFrameworks/MD_GameState.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Subsystems/FogOfWarManager.h"

void UMinimapWIdget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedFogManager();

	AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetOwningPlayer());
	if (!PC) return;
	if (AFogOfWarManager* Fog = AFogOfWarManager::Get(this, (uint8)PC->GetTeam()))
	{
		if (Fog->GetFogTexture())
		{
			InitMinimapFog(Fog->GetFogTexture());
		}
	}
}

void UMinimapWIdget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

	for (auto& Icon : HeroIcons)
	{
		AActor* Hero = Icon.Key;
		if (!Hero || Hero->IsHidden())
		{
			Icon.Value->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}

		// 2. Переводим WorldPos -> UI Pos
		FVector ActorLoc = Hero->GetActorLocation();
		FVector2D NormalizedPos = GetNormalizedPosition(ActorLoc); // (0..1)

		// 3. Умножаем на размер виджета и двигаем иконку
		FVector2D PixelPos = NormalizedPos * MinimapSize;
		Icon.Value->SetRenderTranslation(PixelPos);
		Icon.Value->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMinimapWIdget::CachedFogManager()
{
	AMD_PlayerState* PS = Cast<AMD_PlayerState>(GetOwningPlayerState());

	if (PS)
	{
		FogManager = AFogOfWarManager::Get(this, (uint8)PS->Team);
		if (FogManager)
		{
			MapSize = FogManager->MapSize;
			GridCellSize = FogManager->GridCellSize;
		}
	}
}

void UMinimapWIdget::InitHeroIcons()
{
	AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>();
	if (!GS) return;

	for (const auto& Info : GS->HeroesInfo)
	{
	}
}

FVector2D UMinimapWIdget::GetNormalizedPosition(const FVector& WorldLocation)
{
	// 1. Получаем смещение относительно центра менеджера
	FVector RelativePos = WorldLocation; // - GetActorLocation();

	// 2. Нормализуем относительно общего размера карты (MapSize_Units)
	// MapSize_Units = MapSize.X * GridCellSize (например, 5800)
	float FullMapSize = MapSize.X * GridCellSize;

	// Делим смещение на полный размер и прибавляем 0.5, чтобы перевести диапазон [-0.5..0.5] в [0..1]
	float NormX = (RelativePos.X / FullMapSize) + 0.5f;
	float NormY = (RelativePos.Y / FullMapSize) + 0.5f;

	// 3. Инвертируем Y для UI (в Unreal UI 0,0 — это верхний левый угол, а в мире Y растет вверх)
	// В зависимости от того, как ты заскринил карту, может понадобиться инверсия:
	NormY = 1.0f - NormY;

	// Зажимаем в 0..1, чтобы иконки не вылетали за рамки миникарты
	return FVector2D(FMath::Clamp(NormX, 0.0f, 1.0f), FMath::Clamp(NormY, 0.0f, 1.0f));
}

void UMinimapWIdget::InitMinimapFog(UTexture2D* InFogTexture)
{
	if (MinimapMaterial && InFogTexture)
	{
		MinimapMID = UMaterialInstanceDynamic::Create(MinimapMaterial, this);
		MinimapMID->SetTextureParameterValue(FName("FogMask"), InFogTexture);

		MinimapImage->SetBrushFromMaterial(MinimapMID);

		UE_LOG(LogTemp, Log, TEXT("UMinimapWIdget::InitMinimapFog Successed"));
	}
}