// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_StartupDataBase.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UMD_AbilitySystemComponent;
/**
 *
 */
UCLASS()
class MYDOTA_API UDataAsset_StartupDataBase : public UDataAsset
{
	GENERATED_BODY()

public:

	virtual void GiveToAbilitySystemComponent(UMD_AbilitySystemComponent* InASCToGive, int32 ApplyLevel = 1);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Startup Data")
	TArray<TSubclassOf<UGameplayAbility>> ActivateOnGivenAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Startup Data")
	TArray<TSubclassOf<UGameplayAbility>> ReactiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Startup Data")
	TArray<TSubclassOf<UGameplayEffect>> StartupGameplayEffects;

	void GrandAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilitiesToGive, UMD_AbilitySystemComponent* InASCToGive, int32 ApplyLevel = 1);
};
