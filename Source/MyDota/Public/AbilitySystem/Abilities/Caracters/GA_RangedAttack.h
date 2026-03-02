// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Attacks/GA_AttackBase.h"
#include "GA_RangedAttack.generated.h"

class AMD_ProjectileBase;
/**
 * 
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_RangedAttack : public UGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UGA_RangedAttack(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void OnHitEventReceived(FGameplayEventData Payload) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AMD_ProjectileBase> ProjectileClass;
};
