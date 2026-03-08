// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/HUD/HeroIconWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"

void UHeroIconWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHeroIconWidget::SetHeroIcon(UTexture2D* IconTexture)
{
	if (HeroIcon && IconTexture)
	{
		HeroIcon->SetBrushFromTexture(IconTexture);
	}
}

void UHeroIconWidget::SetTeamColor(bool bIsFriendly)
{
	if (BorderIcon)
	{
		BorderIcon->SetBrushColor(bIsFriendly ? FriendlyColor : EnemyColor);
	}
}

void UHeroIconWidget::SetPinged(bool bPinged)
{
	if (BorderIcon && bPinged)
	{
		BorderIcon->SetBrushColor(PingedColor);
	}
}