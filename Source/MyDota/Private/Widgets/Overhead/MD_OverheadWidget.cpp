// Rail Agliullin Dev. All Rights Reserved


#include "Widgets/Overhead/MD_OverheadWidget.h"

#include "Components/ProgressBar.h"

void UMD_OverheadWidget::UpdateStats(float HP, float MaxHP, float MP, float MaxMP)
{
	if (HealthBar)
	{
		const float HealthPercent = (MaxHP > 0.f) ? (HP / MaxHP) : 0.f;
		HealthBar->SetPercent(HealthPercent);
	}
	if (ManaBar)
	{
		const float ManaPercent = (MaxMP > 0.f) ? (MP / MaxMP) : 0.f;
		ManaBar->SetPercent(ManaPercent);
	}
}
