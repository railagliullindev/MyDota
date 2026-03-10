// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/MD_AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/GameplayEffects/GE_HeroDeathEffect.h"
#include "Characters/MD_CharacterBase.h"
#include "Net/UnrealNetwork.h"

UMD_AttributeSet::UMD_AttributeSet()
{
	InitHealth(1.f);
	InitHealthMax(1.f);
	InitHealthRegen(1.f);
	InitMana(1.f);
	InitManaMax(1.f);
	InitManaRegen(1.f);
}

void UMD_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetHealthMax());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetManaMax());
	}
}

void UMD_AttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetHealthAttribute() && NewValue <= 0.0f)
	{
		// Если мы еще не пометили героя мертвым локально
		if (!GetOwningAbilitySystemComponent()->HasMatchingGameplayTag(MyDotaTags::Status_Death))
		{
			UE_LOG(LogTemp, Warning, TEXT("PostAttributeChange Hero DEAD"));
			// Вызываем визуал смерти не дожидаясь окончания всех серверных расчетов
			OnHeroDied.Broadcast();
		}
	}
}

void UMD_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float CurrentHealth = GetHealth();
		if (CurrentHealth <= 0.0f)
		{
			if (!GetOwningAbilitySystemComponent()->HasMatchingGameplayTag(MyDotaTags::Status_Death))
			{
				UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();

				// Создаем спеку напрямую из C++ класса
				FGameplayEffectSpecHandle DeathSpec = ASC->MakeOutgoingSpec(UGE_HeroDeathEffect::StaticClass(), 1.f, ASC->MakeEffectContext());

				if (DeathSpec.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*DeathSpec.Data.Get());
				}

				// Будим абилку
				FGameplayEventData Payload;
				ASC->HandleGameplayEvent(MyDotaTags::Status_Death, &Payload);

				UE_LOG(LogTemp, Warning, TEXT("Hero DEAD Server"));
				OnHeroDied.Broadcast();
			}
		}
	}
}

void UMD_AttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, HealthMax, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, HealthRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, TotalHealthRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, ManaMax, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, ManaRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UMD_AttributeSet, Strength, COND_None, REPNOTIFY_Always);
}

void UMD_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, Health, OldHealth);

	// На клиенте просто следим за изменением.
	// Тэг Status_Death прилетит сам вместе с Gameplay Effect от сервера.
	if (OldHealth.GetCurrentValue() > 0.f && Health.GetCurrentValue() <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hero DEAD OnRep_Health Clients"))
		// Оповещаем (на клиенте)
		OnHeroDied.Broadcast();
	}

	float NewHealth = Health.GetCurrentValue();
	AActor* Avatar = GetOwningAbilitySystemComponent()->GetAvatarActor();
	AMD_CharacterBase* HeroPawn = Cast<AMD_CharacterBase>(Avatar);

	if (HeroPawn)
	{
		if (OldHealth.GetCurrentValue() <= 0.f && NewHealth > 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hero RESPAWN"))
			// Герой воскрес! Вызываем визуальную починку на клиенте
			HeroPawn->OnRespawnAction(HeroPawn->GetActorLocation());
		}
	}
}

void UMD_AttributeSet::OnRep_HealthMax(const FGameplayAttributeData& OldHealthMax)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, HealthMax, OldHealthMax)
}

void UMD_AttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& OldHealthRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, HealthRegen, OldHealthRegen)
}

void UMD_AttributeSet::OnRep_TotalHealthRegen(const FGameplayAttributeData& OldTotalHealthRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, TotalHealthRegen, OldTotalHealthRegen)
}

void UMD_AttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, Mana, OldMana)
}

void UMD_AttributeSet::OnRep_ManaMax(const FGameplayAttributeData& OldManaMax)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, ManaMax, OldManaMax)
}

void UMD_AttributeSet::OnRep_ManaRegen(const FGameplayAttributeData& OldManaRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, ManaRegen, OldManaRegen)
}

void UMD_AttributeSet::OnRep_AttackRange(const FGameplayAttributeData& OldAttackRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, AttackRange, OldAttackRange)
}

void UMD_AttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, AttackSpeed, OldAttackSpeed)
}

void UMD_AttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMD_AttributeSet, Strength, OldStrength)
}
