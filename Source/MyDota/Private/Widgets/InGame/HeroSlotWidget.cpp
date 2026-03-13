// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/HeroSlotWidget.h"

void UHeroSlotWidget::UpdateSlot(const FPlayerTeamInfo& PlayerInfo, bool bIsLocalPlayer)
{
	CurrentPlayerInfo = PlayerInfo;
	bIsLocalPlayerSlot = bIsLocalPlayer;
}

void UHeroSlotWidget::ClearSlot()
{
}