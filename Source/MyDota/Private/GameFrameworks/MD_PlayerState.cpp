// Rail Agliullin Dev. All Rights Reserved

#include "GameFrameworks/MD_PlayerState.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "Net/UnrealNetwork.h"

AMD_PlayerState::AMD_PlayerState()
{

	AbilitySystemComponent = CreateDefaultSubobject<UMD_AbilitySystemComponent>(TEXT("AbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Mixed режим идеален для PlayerState в MOBA
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMD_AttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AMD_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UMD_AttributeSet* AMD_PlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void AMD_PlayerState::OnRep_Team()
{
	// TODO: Что бы клиент мгновенно узнал о нахначенной стороне
}

void AMD_PlayerState::OnRep_HeroId()
{
	if (!HeroInfoData)
	{
		UE_LOG(LogTemp, Error, TEXT("Forget assign HeroInfoData in AMD_PlayerState"));
		return;
	}

	if (HeroInfoData->HeroesInfo.IsValidIndex(HeroId))
	{
		SelectedHeroClass = HeroInfoData->HeroesInfo[HeroId].HeroClass;
		UE_LOG(LogTemp, Log, TEXT("PS: setup HeroClass success"));
	}
}

EMDTeam AMD_PlayerState::GetTeam() const
{
	return Team;
}

void AMD_PlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMD_PlayerState, HeroId);
	DOREPLIFETIME(AMD_PlayerState, Team);
}
