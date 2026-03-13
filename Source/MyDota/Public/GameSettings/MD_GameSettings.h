// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MD_GameSettings.generated.h"

class UMDHeroInfoDataAsset;
/**
 *
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Dota Game Settings"))
class MYDOTA_API UMD_GameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Database")
	TSoftObjectPtr<UMDHeroInfoDataAsset> HeroInfoDataAsset;

	static const UMD_GameSettings* Get()
	{
		return GetDefault<UMD_GameSettings>();
	}
};
