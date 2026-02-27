// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "GA_HUDManager.generated.h"

class UMD_AttributeSet;
class UMD_MainHUD;
/**
 * 
 */
UCLASS()
class MYDOTA_API UGA_HUDManager : public UMyDotaGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_HUDManager();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMD_MainHUD> MainHUDClass;
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY()
	UMD_MainHUD* MainHUD;
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnManaChanged(const FOnAttributeChangeData& Data);
	
private:
	UPROPERTY()
	UAbilitySystemComponent* ASC;
	
	UPROPERTY()
	const UMD_AttributeSet* AS;
	
	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
};
