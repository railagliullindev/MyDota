// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerWidget.generated.h"

class UTextBlock;
class AMD_GameState;
/**
 *
 */
UCLASS()
class MYDOTA_API UTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;

protected:

	virtual void NativeConstruct() override;

	UFUNCTION()
	virtual void OnGameTimeChanged();

private:

	FTimerHandle TimerHandle;
	float GameTargetTime = -1;
	float CurrentGameTime = -1;

	virtual void TimeTick();

	UPROPERTY()
	AMD_GameState* CachedGameState;
};
