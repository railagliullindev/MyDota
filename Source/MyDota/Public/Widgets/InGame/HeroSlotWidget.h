// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MyDotaStructTypes.h"
#include "Blueprint/UserWidget.h"
#include "HeroSlotWidget.generated.h"

class UTextBlock;
struct FPlayerTeamInfo;
/**
 * Виджет для одного слота героя в интерфейсе
 * Содержит иконку, фон, индикаторы и т.д.
 */
UCLASS()
class MYDOTA_API UHeroSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Обновить слот данными игрока
	UFUNCTION(BlueprintCallable, Category = "Hero Slot")
	void UpdateSlot(const FPlayerTeamInfo& PlayerInfo, bool bIsLocalPlayer);

	// Очистить слот
	UFUNCTION(BlueprintCallable, Category = "Hero Slot")
	void ClearSlot();

	// Установить иконку героя
	UFUNCTION(BlueprintImplementableEvent, Category = "Hero Slot")
	void SetHeroIcon(UTexture2D* Icon);

	// Получить данные слота
	UFUNCTION(BlueprintPure, Category = "Hero Slot")
	const FPlayerTeamInfo& GetPlayerInfo() const
	{
		return CurrentPlayerInfo;
	}

	UFUNCTION(BlueprintPure, Category = "Hero Slot")
	bool IsEmpty() const
	{
		return CurrentPlayerInfo.PlayerId == -1;
	}

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Slot")
	FPlayerTeamInfo CurrentPlayerInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Hero Slot")
	bool bIsLocalPlayerSlot = false;
};
