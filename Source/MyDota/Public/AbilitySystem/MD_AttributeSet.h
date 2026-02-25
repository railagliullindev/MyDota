// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MD_AttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class MYDOTA_API UMD_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UMD_AttributeSet();
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "MainStats")
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS(UMD_AttributeSet, CurrentHealth);
	
	UPROPERTY(BlueprintReadOnly, Category = "MainStats")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMD_AttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "MainStats")
	FGameplayAttributeData CurrentMana;
	ATTRIBUTE_ACCESSORS(UMD_AttributeSet, CurrentMana);
	
	UPROPERTY(BlueprintReadOnly, Category = "MainStats")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UMD_AttributeSet, MaxMana);
	
};
