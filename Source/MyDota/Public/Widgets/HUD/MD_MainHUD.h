// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MD_MainHUD.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class MYDOTA_API UMD_MainHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpdateHealth(float NewValue, float MaxValue);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpdateMana(float NewValue, float MaxValue);
};
