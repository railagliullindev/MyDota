// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroStatusWidget.generated.h"

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

	UPROPERTY(EditAnywhere)
	int32 PlayerIndex;

protected:

	virtual void NativeConstruct() override;

	// Привязка UI компонентов (имена должны совпадать с именами в Blueprint)
	UPROPERTY(meta = (BindWidget))
	UImage* HeroImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RespawnTimerText;

	UPROPERTY(meta = (BindWidget))
	UBorder* DeadOverlay; // Полупрозрачный черный фон под таймером

	UPROPERTY()
	AMD_PlayerState* PS;

	// Настройка визуальных эффектов (Сепия/Десатурация)
	UFUNCTION()
	void UpdateDeathVisuals(bool bIsDead, float SecondsLeft);

	void TickTimer();

private:

	FTimerHandle DeadOverlayTimer;
	int RespawnTime;
};
