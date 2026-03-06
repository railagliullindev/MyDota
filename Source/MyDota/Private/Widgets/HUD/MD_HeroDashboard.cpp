// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/HUD/MD_HeroDashboard.h"

#include "GameplayEffectTypes.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Components/ProgressBar.h"
#include "Controllers/MD_PlayerController.h"
#include "GameFrameworks/MD_PlayerState.h"

void UMD_HeroDashboard::NativeConstruct()
{
	Super::NativeConstruct();

	AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetOwningPlayer());
	if (!PC) return;

	auto* PS = PC->GetPlayerState<AMD_PlayerState>();
	if (!PS) return;

	auto ASC = PS->GetAbilitySystemComponent();
	AS = PS->GetAttributeSet();

	if (ASC && AS)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddUObject(this, &UMD_HeroDashboard::UpdateHealth);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetManaAttribute()).AddUObject(this, &UMD_HeroDashboard::UpdateMana);
	}
}

void UMD_HeroDashboard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMD_HeroDashboard::UpdateHealth(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (HealthBar)
	{
		const float HealthPercent = AS->GetHealth() / AS->GetHealthMax();
		HealthBar->SetPercent(HealthPercent);
	}
}

void UMD_HeroDashboard::UpdateMana(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (ManaBar)
	{
		const float ManaPercent = AS->GetMana() / AS->GetManaMax();
		ManaBar->SetPercent(ManaPercent);
	}
}
