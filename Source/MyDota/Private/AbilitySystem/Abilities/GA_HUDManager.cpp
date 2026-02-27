// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/GA_HUDManager.h"

#include "AbilitySystem/MD_AttributeSet.h"
#include "Controllers/MD_PlayerController.h"
#include "Widgets/HUD/MD_MainHUD.h"

UGA_HUDManager::UGA_HUDManager()
{
	AbilityActivationPolicy = EMyDOtaAbilityActivationPolicy::OnGiven;
}

void UGA_HUDManager::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetActorInfo().PlayerController.Get());
	
	if (PC && MainHUDClass)
	{
		MainHUD = CreateWidget<UMD_MainHUD>(PC, MainHUDClass);
		MainHUD->AddToViewport();
		
		ASC = GetAbilitySystemComponentFromActorInfo();
		AS = ASC->GetSet<UMD_AttributeSet>();
		
		if (ASC && AS)
		{
			HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddUObject(this, &UGA_HUDManager::OnHealthChanged);
			ManaChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(AS->GetManaAttribute()).AddUObject(this, &UGA_HUDManager::OnManaChanged);
			
			FOnAttributeChangeData Data = FOnAttributeChangeData();
			OnHealthChanged(Data);
			OnManaChanged(Data);
		}
	}
}

void UGA_HUDManager::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MainHUD)
	{
		MainHUD->RemoveFromParent();
	}
		
	if (ASC && AS)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetManaAttribute()).Remove(ManaChangedDelegateHandle);
	
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HUDManager::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (MainHUD)
	{
		AS = ASC->GetSet<UMD_AttributeSet>();
		MainHUD->UpdateHealth(AS->GetHealth(), AS->GetHealthMax());
	}
}

void UGA_HUDManager::OnManaChanged(const FOnAttributeChangeData& Data)
{
	if (MainHUD)
	{
		AS = ASC->GetSet<UMD_AttributeSet>();
		MainHUD->UpdateMana(AS->GetMana(), AS->GetManaMax());
	}
}
