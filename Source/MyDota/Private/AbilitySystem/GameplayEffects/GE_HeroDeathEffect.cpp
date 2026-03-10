// Rail Agliullin Dev. All Rights Reserved

#include "AbilitySystem/GameplayEffects/GE_HeroDeathEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "MD_GameplayTags.h"

UGE_HeroDeathEffect::UGE_HeroDeathEffect(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 1. Создаем компонент вручную с ЯВНЫМ именем (чтобы избежать Fatal Error)
	UTargetTagsGameplayEffectComponent* TargetTagsComponent = Initializer.CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(this, TEXT("DeathStatusTagsComponent"));

	if (TargetTagsComponent)
	{
		// 2. Добавляем его в массив компонентов эффекта
		GEComponents.Add(TargetTagsComponent);

		// 3. Настраиваем теги
		FInheritedTagContainer TagContainer;
		TagContainer.AddTag(MyDotaTags::Status_Death);

		TargetTagsComponent->SetAndApplyTargetTagChanges(TagContainer);
	}
}