// Rail Agliullin Dev. All Rights Reserved

#include "Widgets/HUD/MD_HeroDashboard.h"

#include "GameplayEffectTypes.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
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
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetTotalHealthRegenAttribute()).AddUObject(this, &UMD_HeroDashboard::UpdateHealthRegen);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetManaAttribute()).AddUObject(this, &UMD_HeroDashboard::UpdateMana);
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetManaRegenAttribute()).AddUObject(this, &UMD_HeroDashboard::UpdateManaRegen);

		// 1. Подписываемся на изменения
		ASC->GetGameplayAttributeValueChangeDelegate(AS->GetHealthRegenAttribute()).AddUObject(this, &UMD_HeroDashboard::UpdateHealthRegen);

		const FOnAttributeChangeData FakeAttributeChangeData;
		UpdateHealth(FakeAttributeChangeData);
		UpdateHealthRegen(FakeAttributeChangeData);
		UpdateMana(FakeAttributeChangeData);
		UpdateManaRegen(FakeAttributeChangeData);
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

		UE_LOG(LogTemp, Warning, TEXT("Health Updated: %f / %f"), AS->GetHealth(), AS->GetHealthMax());

		CurrentHealthText->SetText(FText::FromString(FormatValue(AS->GetHealth(), AS->GetHealthMax())));
	}
}

void UMD_HeroDashboard::UpdateMana(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (ManaBar)
	{
		const float ManaPercent = AS->GetMana() / AS->GetManaMax();
		ManaBar->SetPercent(ManaPercent);

		CurrentManaText->SetText(FText::FromString(FormatValue(AS->GetMana(), AS->GetManaMax())));
	}
}

void UMD_HeroDashboard::UpdateHealthRegen(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (HealthRegenText)
	{
		HealthRegenText->SetText(FText::FromString(FormatRegen(AS->GetTotalHealthRegen())));
	}
}

void UMD_HeroDashboard::UpdateManaRegen(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (ManaRegenText)
	{
		ManaRegenText->SetText(FText::FromString(FormatRegen(AS->GetManaRegen())));
	}
}

FString UMD_HeroDashboard::FormatValue(float ValueA, float ValueB) const
{
	return FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(ValueA), FMath::RoundToInt(ValueB));
}

FString UMD_HeroDashboard::FormatRegen(float Value) const
{
	// Форматируем регенерацию с одним знаком после запятой
	// В Dota 2 обычно показывают +3.4 или -2.1
	float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;
	FString Sign = RoundedValue >= 0 ? TEXT("+") : TEXT("");

	return FString::Printf(TEXT("%s%.1f"), *Sign, RoundedValue);
}