// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/InGame/HeroSlotWidget.h"

#include "Components/TextBlock.h"

void UHeroSlotWidget::UpdateSlot(const FPlayerTeamInfo& PlayerInfo, bool bIsLocalPlayer)
{
	CurrentPlayerInfo = PlayerInfo;
	bIsLocalPlayerSlot = bIsLocalPlayer;

	// FString NetPrefix = GetWorld()->GetNetMode() == NM_Client ? FString::Printf(TEXT("Client %d"), UE::GetPlayInEditorID()) : TEXT("Server");

	PlayerName->SetText(FText::FromString(PlayerInfo.PlayerName));
}

void UHeroSlotWidget::ClearSlot()
{
}