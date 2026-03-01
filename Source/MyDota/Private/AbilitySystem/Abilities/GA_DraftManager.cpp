// Rail Agliullin Dev. All Rights Reserved


#include "AbilitySystem/Abilities/GA_DraftManager.h"

#include "AbilitySystemComponent.h"
#include "MD_GameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "Controllers/MD_PlayerController.h"
#include "Pawns/MD_CameraPawn.h"

UGA_DraftManager::UGA_DraftManager()
{
	SetAssetTags(FGameplayTagContainer{MyDotaTags::Ability_ShowDraft});
}

void UGA_DraftManager::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Activate Draft Ability"));
	
	PC = Cast<AMD_PlayerController>(GetActorInfo().PlayerController.Get());
	
	if (PC && DraftWidgetClass)
	{
		// 1. Открываем UI
		DraftWidget = CreateWidget<UUserWidget>(PC, DraftWidgetClass);
		DraftWidget->AddToViewport();
		
		// 2. Настраиваем ввод
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(DraftWidget->GetCachedWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
		
		// 3. Блокируем CameraPawn
		if (AMD_CameraPawn* CamPawn = Cast<AMD_CameraPawn>(PC->GetPawn()))
		{
			CamPawn->GetAbilitySystemComponent()->AddLooseGameplayTag(MyDotaTags::Camera_Locked);
		}
	}
}

void UGA_DraftManager::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("End Draft Ability"));
	if (DraftWidget)
	{
		DraftWidget->RemoveFromParent();
	}
	
	
	if (PC)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
		
		if (AMD_CameraPawn* CamPawn = Cast<AMD_CameraPawn>(PC->GetPawn()))
		{
			CamPawn->GetAbilitySystemComponent()->RemoveLooseGameplayTag(MyDotaTags::Camera_Locked);
		}
	}
	
	
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
