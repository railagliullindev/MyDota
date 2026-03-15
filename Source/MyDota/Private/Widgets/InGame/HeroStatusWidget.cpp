// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/HeroStatusWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "GameFramework/PlayerState.h"
#include "GameFrameworks/MD_GameState.h"
#include "GameFrameworks/MD_PlayerState.h"
#include "Subsystems/MD_DataSubsystem.h"

void UMD_HeroIconWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// TODO: Пересобрать решение

	auto GS = Cast<AMD_GameState>(GetWorld()->GetGameState());

	for (auto Element : GS->PlayersInfo)
	{
		if (Element.GetTeam() == Team)
		{
			if (Element.Slot == SlotIndex)
			{
				PlayerInfo = Element;
				break;
			}
		}
	}

	for (auto Player : GS->PlayerArray)
	{
		if (Player->GetPlayerId() == PlayerInfo.PlayerId)
		{
			// Например, берем 2-го игрока из списка для этой конкретной иконки
			PS = Cast<AMD_PlayerState>(Player);
			break;
		}
	}

	if (PS)
	{
		PS->OnRespawnStatusChanged.AddDynamic(this, &UMD_HeroIconWidget::UpdateDeathVisuals);
	}

	Init();
}

void UMD_HeroIconWidget::Init() const
{
	if (!PS) return;

	PLayerNameText->SetText(FText::FromString(PlayerInfo.PlayerName));

	if (auto* DB = UMD_DataSubsystem::Get(this))
	{
		auto HeroInfo = DB->GetHeroInfo(PS->HeroId);
		if (HeroInfo.IsValid())
		{
			HeroImage->SetBrushFromTexture(HeroInfo.HeroIcon);
		}
	}
}

void UMD_HeroIconWidget::UpdateDeathVisuals(bool bIsDead, float SecondsLeft)
{
	if (bIsDead)
	{
		RespawnTime = SecondsLeft - 1;

		// Показываем текст таймера
		RespawnTimerText->SetText(FText::AsNumber(RespawnTime));

		if (DeadOverlay) DeadOverlay->SetVisibility(ESlateVisibility::Visible);

		// Эффект Dota 2: иконка становится черно-белой
		HeroImage->SetColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

		GetWorld()->GetTimerManager().SetTimer(DeadOverlayTimer, this, &UMD_HeroIconWidget::TickTimer, 1.f, true);
	}
	else
	{
		// Прячем всё лишнее
		if (DeadOverlay) DeadOverlay->SetVisibility(ESlateVisibility::Collapsed);

		HeroImage->SetColorAndOpacity(FLinearColor::White);

		GetWorld()->GetTimerManager().ClearTimer(DeadOverlayTimer);
	}
}

void UMD_HeroIconWidget::TickTimer()
{
	if (RespawnTimerText)
	{
		RespawnTime -= 1;
		RespawnTimerText->SetText(FText::AsNumber(RespawnTime));
	}
}