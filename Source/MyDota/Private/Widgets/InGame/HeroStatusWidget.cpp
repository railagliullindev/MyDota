// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/HeroStatusWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "GameFrameworks/MD_GameState.h"
#include "GameFrameworks/MD_PlayerState.h"

void UMD_HeroIconWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>())
	{
		const bool bIsRadiant = PlayerIndex < 5 ? true : false;
		const int32 Team = bIsRadiant ? 2 : 1;
		int32 Iter = bIsRadiant ? PlayerIndex : PlayerIndex - 5;

		int32 PlayerID = -1;
		for (const auto& Element : GS->HeroesInfo)
		{
			// 0 - none, 1 - Dire, 2 - Radian
			if (Element.TeamId == Team)
			{
				if (Iter == 0)
				{
					PlayerID = Element.PlayerId;
					break;
				}
				else
				{
					Iter -= 1;
				}
			}
		}

		if (PlayerID == -1) return;

		for (auto Player : GS->PlayerArray)
		{
			if (Player->GetPlayerId() == PlayerID)
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