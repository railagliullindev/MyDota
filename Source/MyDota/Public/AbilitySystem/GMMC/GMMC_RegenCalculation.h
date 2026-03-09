// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "GMMC_RegenCalculation.generated.h"

/**
 *
 */
UCLASS()
class MYDOTA_API UGMMC_RegenCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:

	UGMMC_RegenCalculation();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

	UPROPERTY(EditDefaultsOnly)
	float StrengthPercentForHeal = 0.06f;
};
