// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroIconWidget.generated.h"

class UBorder;
class UImage;
/**
 *
 */
UCLASS()
class MYDOTA_API UHeroIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetHeroIcon(UTexture2D* IconTexture);

	UFUNCTION(BlueprintCallable)
	void SetTeamColor(bool bIsFriendly);

	UFUNCTION(BlueprintCallable)
	void SetPinged(bool bPinged);

protected:

	UPROPERTY(meta = (BindWidget))
	UImage* HeroIcon;

	UPROPERTY(meta = (BindWidget))
	UBorder* BorderIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FLinearColor FriendlyColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FLinearColor EnemyColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	FLinearColor PingedColor = FLinearColor::Yellow;
};
