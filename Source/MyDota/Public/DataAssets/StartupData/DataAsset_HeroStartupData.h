// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DataAsset_StartupDataBase.h"
#include "DataAsset_HeroStartupData.generated.h"

struct FMDHeroAbilitySet;
/**
 * 
 */
UCLASS()
class MYDOTA_API UDataAsset_HeroStartupData : public UDataAsset_StartupDataBase
{
	GENERATED_BODY()
	
public:
	
	virtual void GiveToAbilitySystemComponent(UMD_AbilitySystemComponent* InASCToGive, int32 ApplyLevel = 1) override;
	
private:
	
	UPROPERTY(EditDefaultsOnly, Category="Startup Data", meta = (TitleProperty = "InputTag"))
	TArray<FMDHeroAbilitySet> HeroStartupAbilitySets;
};
