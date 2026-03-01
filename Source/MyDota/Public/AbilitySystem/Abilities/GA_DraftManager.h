// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "GA_DraftManager.generated.h"

class AMD_PlayerController;
/**
 * 
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_DraftManager : public UMyDotaGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UGA_DraftManager();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DraftWidgetClass;
	
protected:
	
	UPROPERTY()
	UUserWidget* DraftWidget;
	
	UPROPERTY()
	AMD_PlayerController* PC;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
