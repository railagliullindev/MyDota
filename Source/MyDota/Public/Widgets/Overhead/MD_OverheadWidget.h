// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MD_OverheadWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UMD_OverheadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;
	
	void UpdateStats(float HP, float MaxHP, float MP, float MaxMP);
};
