// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/MD_CharacterBase.h"
#include "MinimapWIdget.generated.h"

class AMD_PlayerController;
class UCanvasPanel;
enum class EMDTeam : uint8;
class AMD_GameState;
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
	virtual void NativeDestruct() override;

protected:

	void CachedFogManager();
	void InitHeroIcons();
	void UpdateHeroIcon(AActor* InActor, UUserWidget* IconWidget);
	FVector2D GetNormalizedPosition(const FVector& WorldLocation);
	void OnVisibilityChanged(AActor* InActor, bool IsVisible);
	void OnHeroDied(AActor* InActor);
	void ClearHeroIcons();

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

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MinimapCanvas;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* MinimapMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* MinimapMID;

	UPROPERTY(EditAnywhere, Category = "Setup")
	FVector2D MinimapSizeInWidget = FVector2D(300.f, 300.f);

private:

	void InitMinimapFog(UTexture2D* InFogTexture);
	UUserWidget* CreateIconForHero(AMD_CharacterBase* InHero);

	UPROPERTY()
	AMD_GameState* GS;

	UPROPERTY()
	AMD_PlayerController* PC;

	EMDTeam LocalTeam;

	UPROPERTY()
	TMap<AActor*, UUserWidget*> HeroIcons;

	FIntPoint MapSize;
	float GridCellSize;
	FVector2D WorldMapSize;

	// Для отслеживания умерших героев
	UPROPERTY()
	TSet<AActor*> DeadHeroes;

	FTimerHandle InitTimerHandle;
};
