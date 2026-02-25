// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/MD_AttributeSet.h"
#include "GameplayEffectExtension.h"

UMD_AttributeSet::UMD_AttributeSet()
{
	InitCurrentHealth(1.f);
	InitMaxHealth(1.f);
	InitCurrentMana(1.f);
	InitMaxMana(1.f);
}

void UMD_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth());
		SetCurrentHealth(NewCurrentHealth);
	}
	
	if (Data.EvaluatedData.Attribute == GetCurrentManaAttribute())
	{
		const float NewCurrentMana = FMath::Clamp(GetCurrentMana(), 0.f, GetMaxMana());
		SetCurrentMana(NewCurrentMana);
	}
	
}
