// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "MyDotaStructTypes.h"
#include "Blueprint/UserWidget.h"
#include "HeroStatusWidget.generated.h"

enum class EMDTeam : uint8;
class AMD_PlayerState;
class UBorder;
class UTextBlock;
class UImage;
/**
 *
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UMD_HeroIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Setup")
	int32 SlotIndex;

	UPROPERTY(EditAnywhere, Category = "Setup")
	EMDTeam Team;

protected:

	virtual void NativeConstruct() override;

	// Привязка UI компонентов (имена должны совпадать с именами в Blueprint)
	UPROPERTY(meta = (BindWidget))
	UImage* HeroImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PLayerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RespawnTimerText;

	UPROPERTY(meta = (BindWidget))
	UBorder* DeadOverlay; // Полупрозрачный черный фон под таймером

	UPROPERTY()
	AMD_PlayerState* PS;

	void Init() const;

	// Настройка визуальных эффектов (Сепия/Десатурация)
	UFUNCTION()
	void UpdateDeathVisuals(bool bIsDead, float SecondsLeft);

	void TickTimer();

private:

	FTimerHandle DeadOverlayTimer;
	int RespawnTime;

	FPlayerTeamInfo PlayerInfo;
};
