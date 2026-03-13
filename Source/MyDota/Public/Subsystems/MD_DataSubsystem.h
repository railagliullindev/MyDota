// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MD_DataSubsystem.generated.h"

class UMDHeroInfoDataAsset;
struct FHeroInfo;

/**
 * Глобальная база данных (синглтон), доступная на Сервере и Клиенте.
 * Живет на протяжении всего времени работы игры.
 */
UCLASS()
class MYDOTA_API UMD_DataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Статический помощник для получения субсистемы из любого места C++
	 * Использование: UMD_DataSubsystem::Get(GetWorld())->GetHero(5);
	 */
	static UMD_DataSubsystem* Get(const UObject* WorldContextObject);

	FHeroInfo GetHeroInfo(const int32 InHeroID);
	FHeroInfo GetHeroInfo(const AActor* InActor);

protected:

	UPROPERTY()
	TObjectPtr<UMDHeroInfoDataAsset> CachedHeroInfoDataAsset;
};
