// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystem/Abilities/Caracters/GA_MeleeAttackBase.h"

#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_MeleeAttackBase::UGA_MeleeAttackBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer{MyDotaTags::Ability_Attack});
}

void UGA_MeleeAttackBase::OnHitEventReceived(FGameplayEventData Payload)
{
	if (!MyTarget.IsValid()) return;

	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());

	if (DamageSpec.IsValid())
	{
		// 2. Накладываем эффект на цель (MyTarget)
		// Используем ApplyGameplayEffectSpecToTarget для мультиплеерной надежности
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageSpec, GetAbilityTargetData());
	}
}
