// Rail Agliullin Dev. All Rights Reserved

#include "Subsystems/MD_DataSubsystem.h"

#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "GameSettings/MD_GameSettings.h"

void UMD_DataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Берем путь из настроек
	if (const UMD_GameSettings* Settings = UMD_GameSettings::Get())
	{
		// Загружаем ассет синхронно (так как это старт игры)
		CachedHeroInfoDataAsset = Cast<UMDHeroInfoDataAsset>(Settings->HeroInfoDataAsset.LoadSynchronous());

		if (CachedHeroInfoDataAsset)
		{
			UE_LOG(LogTemp, Log, TEXT("Database: DataAsset Loaded Successfully!"));
		}
	}
}

UMD_DataSubsystem* UMD_DataSubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				return GI->GetSubsystem<UMD_DataSubsystem>();
			}
		}
	}
	return nullptr;
}

FHeroInfo UMD_DataSubsystem::GetHeroInfo(const int32 InHeroID)
{
	if (CachedHeroInfoDataAsset.Get()->HeroesInfo.IsValidIndex(InHeroID))
	{
		return CachedHeroInfoDataAsset.Get()->HeroesInfo[InHeroID];
	}

	return FHeroInfo();
}

FHeroInfo UMD_DataSubsystem::GetHeroInfo(const AActor* InActor)
{
	if (!InActor) return FHeroInfo();

	return CachedHeroInfoDataAsset.Get()->GetHeroInfoDataAsset(InActor);
}