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
	FName HeroName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AMD_CharacterBase> HeroClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* HeroIcon;

	FHeroInfo()
		: HeroName(NAME_None)
		, HeroIcon(nullptr)
	{
	}
};

UCLASS(BlueprintType)
class MYDOTA_API UMDHeroInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Info")
	TArray<FHeroInfo> HeroesInfo;

	UTexture2D* GetIconForHero(AActor* InHero) const
	{
		if (!InHero) return nullptr;
		for (const auto& Info : HeroesInfo)
		{
			if (InHero->IsA(Info.HeroClass)) return Info.HeroIcon;
		}
		return nullptr;
	};
};
