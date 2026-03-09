// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "MD_HeroDashboard.generated.h"

class UTextBlock;
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
	UTextBlock* CurrentHealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthRegenText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ManaBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentManaText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ManaRegenText;

private:

	void UpdateHealth(const FOnAttributeChangeData& OnAttributeChangeData);
	void UpdateMana(const FOnAttributeChangeData& OnAttributeChangeData);
	void UpdateHealthRegen(const FOnAttributeChangeData& OnAttributeChangeData);
	void UpdateManaRegen(const FOnAttributeChangeData& OnAttributeChangeData);

	UPROPERTY()
	UMD_AttributeSet* AS;

	FString FormatValue(float ValueA, float ValueB) const;
	FString FormatRegen(float Value) const;
};
