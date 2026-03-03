// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "AbilitySystem/Abilities/Attacks/GA_AttackBase.h"
#include "GA_MeleeAttackBase.generated.h"

class UMD_AttributeSet;
class AAIController;
/**
 *
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_MeleeAttackBase : public UGA_AttackBase
{
	GENERATED_BODY()

public:

	UGA_MeleeAttackBase(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void OnHitEventReceived(FGameplayEventData Payload) override;
};
