// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "MyDota/MyDota.h"

void UMyDotaGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (AbilityActivationPolicy == EMyDOtaAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo && !Spec.IsActive())
		{
			const bool result = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
			UE_LOG(LogMyDotaGAS, Warning, TEXT("OnGiven Ability %s is %s"), *GetName(), result ? TEXT("Activated") : TEXT("NON ACTIVATED"));
		}
	}
}

void UMyDotaGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AbilityActivationPolicy == EMyDOtaAbilityActivationPolicy::OnGiven)
	{
		if (ActorInfo)
		{
			ActorInfo->AbilitySystemComponent->ClearAbility(Handle);
		}
	}
}
