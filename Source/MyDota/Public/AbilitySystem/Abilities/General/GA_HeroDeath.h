// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "GA_HeroDeath.generated.h"

/**
 *
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_HeroDeath : public UMyDotaGameplayAbility
{
	GENERATED_BODY()

public:

	UGA_HeroDeath();

protected:

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
		bool bWasCancelled) override;

	void UpdateRespawnTimerOnPlayerState(float RespawnTime);
	void OnRespawnFinished();

	FTimerHandle RespawnTimerHandle;
};
