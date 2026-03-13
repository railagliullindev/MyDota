// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/HeroStatusWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UMD_HeroIconWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// TODO: Пересобрать решение
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