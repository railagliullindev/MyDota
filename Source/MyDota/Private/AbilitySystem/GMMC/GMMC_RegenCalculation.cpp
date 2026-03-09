// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystem/GMMC/GMMC_RegenCalculation.h"

#include "AbilitySystem/MD_AttributeSet.h"

struct FRegenStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseRegenDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;

	FRegenStatics()
	{
		// Захватываем базовый реген
		BaseRegenDef = FGameplayEffectAttributeCaptureDefinition(UMD_AttributeSet::GetHealthRegenAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);

		// Захватываем силу
		StrengthDef = FGameplayEffectAttributeCaptureDefinition(UMD_AttributeSet::GetStrengthAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
	}
};

static const FRegenStatics& RegenStatics()
{
	static FRegenStatics Statics;
	return Statics;
}

UGMMC_RegenCalculation::UGMMC_RegenCalculation()
{
	RelevantAttributesToCapture.Add(RegenStatics().BaseRegenDef);
	RelevantAttributesToCapture.Add(RegenStatics().StrengthDef);
}

float UGMMC_RegenCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	// 1. Получаем базу (HealthRegen)
	float Base = 0.f;
	GetCapturedAttributeMagnitude(RegenStatics().BaseRegenDef, Spec, EvalParams, Base);

	// 2. Получаем бонус от Силы (Strength), если есть
	float Strength = 0.f;
	GetCapturedAttributeMagnitude(RegenStatics().StrengthDef, Spec, EvalParams, Strength);

	// Итог для TotalHealthRegen
	return Base + (Strength * StrengthPercentForHeal);
}