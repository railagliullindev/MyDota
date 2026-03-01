// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "GA_MeleeAttackBase.generated.h"

class UMD_AttributeSet;
class AAIController;
/**
 * 
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_MeleeAttackBase : public UMyDotaGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_MeleeAttackBase();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	
	const UMD_AttributeSet* GetAttributeSet() const;
	
	void RunAttackLoop();
	void MoveToTarget();
	void PlayAttackMontage();
	
	float GetDistanceToTarget() const;
	bool IsFacing() const;
	
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AcceptanceAngle;
	
	// Класс эффекта, который будет наносить урон (выбирается в редакторе)
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;
private:
	
	UFUNCTION()
	void OnAttackMontageFinished();
	
	FGameplayAbilityTargetDataHandle GetAbilityTargetData();
	
	UPROPERTY()
	AAIController* AIC;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> MyTarget;
	
	UPROPERTY()
	FTimerHandle TimerHandle;
	
	float AttackRange;
	float AttackSpeed;
};
