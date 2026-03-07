// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MinimapWIdget.generated.h"

class UImage;
class UMDHeroInfoDataAsset;
class AFogOfWarManager;
/**
 *
 */
UCLASS()
class MYDOTA_API UMinimapWIdget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:

	void CachedFogManager();
	void InitHeroIcons();

	FVector2D GetNormalizedPosition(const FVector& WorldLocation);

	UPROPERTY()
	AFogOfWarManager* FogManager;

	UPROPERTY()
	FVector2D MinimapSize;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	UMDHeroInfoDataAsset* HeroInfoData;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<UUserWidget> IconHeroClass;

	UPROPERTY(meta = (BindWidget))
	UImage* MinimapImage;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* MinimapMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* MinimapMID;

private:

	void InitMinimapFog(UTexture2D* InFogTexture);

	UPROPERTY()
	TMap<AActor*, UUserWidget*> HeroIcons;

	FIntPoint MapSize;
	float GridCellSize;
};
