// Rail Agliullin Dev. All Rights Reserved


#include "DataAssets/StartupData/DataAsset_StartupDataBase.h"

#include "GameplayEffect.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "MyDota/MyDota.h"

void UDataAsset_StartupDataBase::GiveToAbilitySystemComponent(UMD_AbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	check(InASCToGive);
	
	GrandAbilities(ActivateOnGivenAbilities, InASCToGive, ApplyLevel);
	GrandAbilities(ReactiveAbilities, InASCToGive, ApplyLevel);
	
	if (!StartupGameplayEffects.IsEmpty())
	{
		for (const auto& EffectClass : StartupGameplayEffects)
		{
			if (!EffectClass) continue;
			
			UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
			
			InASCToGive->ApplyGameplayEffectToSelf(
				EffectCDO,
				ApplyLevel,
				InASCToGive->MakeEffectContext());
		}
	}
}

void UDataAsset_StartupDataBase::GrandAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilitiesToGive,
                                                UMD_AbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty()) return;
	
	for (const auto& AbilityClass : InAbilitiesToGive)
	{
		if (!AbilityClass) continue;
		
		FGameplayAbilitySpec AbilitySpec(AbilityClass);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		
		
		InASCToGive->GiveAbility(AbilitySpec);
		
		UE_LOG(LogMyDotaGAS, Log, TEXT("%s grand ability %s"), *InASCToGive->GetOwner()->GetName(), *AbilitySpec.Ability.GetName());
	}
}
