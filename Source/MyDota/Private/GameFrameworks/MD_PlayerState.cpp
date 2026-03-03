// Rail Agliullin Dev. All Rights Reserved

#include "GameFrameworks/MD_PlayerState.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "Characters/MD_CharacterBase.h"
#include "GameFrameworks/MD_GameState.h"
#include "Net/UnrealNetwork.h"

void AMD_PlayerState::Server_SetSelectedHero_Implementation(TSubclassOf<AMD_CharacterBase> InHeroClass)
{
	SelectedHeroClass = InHeroClass;

	FString RoleString = HasAuthority() ? TEXT("ListenServer-Host") : TEXT("Remote-Client");
	UE_LOG(LogTemp, Log, TEXT("[%s] выбрал героя - %s"), *RoleString, *SelectedHeroClass.GetDefaultObject()->GetName());

	if (AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>())
	{
		GS->MarkHeroAsPicked(SelectedHeroClass);
	}
}

AMD_PlayerState::AMD_PlayerState()
{

	// Setup Ability components
	AbilitySystemComponent = CreateDefaultSubobject<UMD_AbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed режим идеален для PlayerState в MOBA
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMD_AttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AMD_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UMD_AttributeSet* AMD_PlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void AMD_PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMD_PlayerState, SelectedHeroClass);
	DOREPLIFETIME(AMD_PlayerState, bIsTeamA);
}

bool AMD_PlayerState::Server_SetSelectedHero_Validate(TSubclassOf<AMD_CharacterBase> InHeroClass)
{
	if (InHeroClass == nullptr)
	{
		return false;
	}

	// ПРОВЕРКА 2: Не занят ли этот герой кем-то другим?
	// (Логика Dota 2: один герой на одну команду/игру)
	if (AMD_GameState* GS = GetWorld()->GetGameState<AMD_GameState>())
	{
		if (GS->IsHeroAlreadyPicked(InHeroClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Читер или лаг: Попытка выбрать уже занятого героя!"));
			return false;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Все ок!"));
	// Если всё ок - возвращаем true, и только тогда выполнится _Implementation
	return true;
}
