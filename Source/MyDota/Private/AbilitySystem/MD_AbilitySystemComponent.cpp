// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/MD_AbilitySystemComponent.h"

#include "MD_GameplayTags.h"
#include "MyDota/MyDota.h"

void UMD_AbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	
	if (UGameplayAbility* Ability = AbilitySpec.Ability)
	{
		for (const FGameplayTag& Tag : Ability->GetAssetTags())
		{
			if (Tag.MatchesTag(MyDotaTags::InputTag))
			{
				AbilityTagMap.Add(Tag, AbilitySpec.Handle);
				UE_LOG(LogMyDotaGAS, Log, TEXT("GAS: Абилка %s привязана к тегу %s"), *Ability->GetName(), *Tag.ToString());
				break;
			}
		}
	}
}

void UMD_AbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);
	
	if (UGameplayAbility* Ability = AbilitySpec.Ability)
	{
		for (const FGameplayTag& Tag : Ability->GetAssetTags())
		{
			if (Tag.MatchesTag(MyDotaTags::Ability))
			{
				if (AbilityTagMap.Contains(Tag) && AbilityTagMap[Tag] == AbilitySpec.Handle)
				{
					AbilityTagMap.Remove(Tag);
                    
					UE_LOG(LogMyDotaGAS, Log, TEXT("GAS: Абилка %s удалена, тег %s очищен из мапы"), 
						*Ability->GetName(), 
						*Tag.ToString());
					break;
				}
			}
		}
	}
}

void UMD_AbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InInputTag)
{
	if (!InInputTag.IsValid() && !AbilityTagMap.Contains(InInputTag)) return;
	
	FGameplayAbilitySpecHandle* Handle = AbilityTagMap.Find(InInputTag);
	if (Handle)
	{
		TryActivateAbility(*Handle);
	}
}

void UMD_AbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InInputTag)
{
	if (!InInputTag.IsValid() && !AbilityTagMap.Contains(InInputTag)) return;
	
	FGameplayAbilitySpecHandle* Handle = AbilityTagMap.Find(InInputTag);
	if (Handle)
	{
		CancelAbilityHandle(*Handle);
	}
}
