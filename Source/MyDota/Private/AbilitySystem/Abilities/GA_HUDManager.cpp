// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystem/Abilities/GA_HUDManager.h"

#include "AbilitySystem/MD_AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "Controllers/MD_PlayerController.h"

UGA_HUDManager::UGA_HUDManager()
{
	AbilityActivationPolicy = EMyDOtaAbilityActivationPolicy::OnGiven;
}

void UGA_HUDManager::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetActorInfo().PlayerController.Get());

	if (PC && WidgetClass)
	{
		Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
		Widget->AddToViewport();
	}
}

void UGA_HUDManager::EndAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
