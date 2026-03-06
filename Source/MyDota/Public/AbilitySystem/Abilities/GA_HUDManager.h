// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "GA_HUDManager.generated.h"

class UMD_AttributeSet;
class UMD_HeroDashboard;
/**
 *
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_HUDManager : public UMyDotaGameplayAbility
{
	GENERATED_BODY()

public:

	UGA_HUDManager();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> WidgetClass;

protected:

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UPROPERTY()
	UUserWidget* Widget;
};
