// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Characters/MD_CharacterBase.h"
#include "Engine/DataAsset.h"
#include "MDHeroInfoDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FHeroInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName HeroName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AMD_CharacterBase> HeroClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* HeroIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* HeroMinimapIcon = nullptr;

	bool IsValid() const
	{
		return HeroName != NAME_None;
	}
};

UCLASS(BlueprintType)
class MYDOTA_API UMDHeroInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Info")
	TArray<FHeroInfo> HeroesInfo;

	UTexture2D* GetIconForHero(const AActor* InHero) const
	{
		if (!InHero) return nullptr;
		for (const auto& Info : HeroesInfo)
		{
			if (InHero->IsA(Info.HeroClass)) return Info.HeroIcon;
		}
		return nullptr;
	};

	UTexture2D* GetMinimapIconForHero(const AActor* InHero) const
	{
		if (!InHero) return nullptr;
		for (const auto& Info : HeroesInfo)
		{
			if (InHero->IsA(Info.HeroClass)) return Info.HeroMinimapIcon;
		}
		return nullptr;
	};

	FHeroInfo GetHeroInfoDataAsset(const AActor* InHero) const
	{
		if (!InHero) return FHeroInfo();
		for (const auto& Info : HeroesInfo)
		{
			if (InHero->IsA(Info.HeroClass)) return Info;
		}
		return FHeroInfo();
	}
};
