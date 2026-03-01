// Rail Agliullin Dev. All Rights Reserved


#include "DataAssets/StartupData/DataAsset_HeroStartupData.h"

#include "MyDotaStructTypes.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "MyDota/MyDota.h"

void UDataAsset_HeroStartupData::GiveToAbilitySystemComponent(UMD_AbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);

	for (const auto& AbilitySet : HeroStartupAbilitySets)
	{
		if (!AbilitySet.IsValid()) continue;
		
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);
		
		InASCToGive->GiveAbility(AbilitySpec);
		
		UE_LOG(LogMyDotaGAS, Log, TEXT("%s grand ability %s"), *InASCToGive->GetOwner()->GetName(), *AbilitySpec.Ability.GetName());
	}
}
