// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "MD_HeroDashboard.generated.h"

class UMD_AttributeSet;
class UProgressBar;
/**
 *
 */
UCLASS()
class MYDOTA_API UMD_HeroDashboard : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

	void UpdateHealth(const FOnAttributeChangeData& OnAttributeChangeData);

	void UpdateMana(const FOnAttributeChangeData& OnAttributeChangeData);

private:

	UPROPERTY()
	UMD_AttributeSet* AS;
};
