// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/GA_CameraFollow.h"

#include "MD_GameplayTags.h"

UGA_CameraFollow::UGA_CameraFollow()
{
	FGameplayTagContainer Container;
	Container.AddTag(MyDotaTags::Ability_Camera_FollowingHero);
	Container.AddTag(MyDotaTags::InputTag_FollowToHero);
	
	SetAssetTags(Container);
	
	ActivationOwnedTags.AddTag(MyDotaTags::Camera_FollowingHero);
}

void UGA_CameraFollow::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	/*if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}*/
}

void UGA_CameraFollow::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
