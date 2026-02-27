#pragma once
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"

#include "MyDotaStructTypes.generated.h"

USTRUCT(BlueprintType)
struct FMDHeroAbilitySet
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityToGrant;
	
	bool IsValid() const
	{
		return InputTag.IsValid() && AbilityToGrant;
	}
};
