// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/MyDotaGameplayAbility.h"
#include "GA_AttackBase.generated.h"

class UMD_AttributeSet;
class AAIController;
/**
 * 
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API UGA_AttackBase : public UMyDotaGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_AttackBase(const FObjectInitializer& Initializer);
	
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
	virtual void OnHitEventReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnAttackMontageFinished();
	
	FGameplayAbilityTargetDataHandle GetAbilityTargetData();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AcceptanceAngle;
	
	// Класс эффекта, который будет наносить урон (выбирается в редакторе)
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> MyTarget;
	
	UPROPERTY()
	AAIController* AIC;
	
	float AttackRange;
	float AttackSpeed;
	
private:
	
	UPROPERTY()
	FTimerHandle TimerHandle;
};
