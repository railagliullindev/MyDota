// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "MD_AbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYDOTA_API UMD_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	
	void CancelAbilityWithTag(const FGameplayTag& InTag);
	
	//UFUNCTION(BlueprintCallable, Category = "Ability", meta = (Categories = "InputTag"))
	//bool TryActivateAbilityWithTag(FGameplayTag InInputTag);
	
	void OnAbilityInputPressed(const FGameplayTag& InInputTag);
	void OnAbilityInputReleased(const FGameplayTag& InInputTag);
	
protected:
	UPROPERTY()
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> AbilityTagMap;
};
