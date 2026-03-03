// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/HUD/MD_MainHUD.h"

#include "Components/ProgressBar.h"

void UMD_MainHUD::UpdateHealth(float NewValue, float MaxValue)
{
	if (HealthBar && MaxValue > 0.f)
	{
		const float HealthPercent = NewValue / MaxValue;
		HealthBar->SetPercent(HealthPercent);
	}
}

void UMD_MainHUD::UpdateMana(float NewValue, float MaxValue)
{
	if (ManaBar && MaxValue > 0.f)
	{
		const float ManaPercent = NewValue / MaxValue;
		ManaBar->SetPercent(ManaPercent);
	}
}
