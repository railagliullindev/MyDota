// Rail Agliullin Dev. All Rights Reserved

#include "GameFrameworks/MD_PlayerState.h"

#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "AbilitySystem/MD_AttributeSet.h"
#include "DataAssets/HeroInfo/MDHeroInfoDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/MD_DataSubsystem.h"

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
	// Клиент узнает о своей команде
	UE_LOG(LogTemp, Log, TEXT("Player %s team is now %d"), *GetPlayerName(), (uint8)Team);

	// Можно обновить цвет UI, миникарту и т.д.
	// OnTeamChanged.Broadcast(Team);
}

void AMD_PlayerState::OnRep_RespawnTimeFinished()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Если время в будущем — герой мертв
	// const bool bIsDead = RespawnTimeFinished > CurrentTime;

	const bool bIsDead = (RespawnTimeFinished > 0) ? true : false;

	int32 SecondsLeft = 0;
	if (bIsDead)
	{
		// Вычисляем разницу и округляем вверх
		SecondsLeft = FMath::CeilToInt(RespawnTimeFinished - CurrentTime);
	}

	// Рассылаем статус (теперь и сервер, и клиенты получат одинаковые данные)
	OnRespawnStatusChanged.Broadcast(bIsDead, SecondsLeft);
}

void AMD_PlayerState::OnRep_HeroId()
{
	const auto DataSubsystem = UMD_DataSubsystem::Get(this);
	if (!DataSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Forget assign HeroInfoData in AMD_PlayerState"));
		return;
	}

	FHeroInfo HeroInfo = DataSubsystem->GetHeroInfo(HeroId);
	if (HeroInfo.IsValid())
	{
		SelectedHeroClass = HeroInfo.HeroClass;
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
	DOREPLIFETIME(AMD_PlayerState, RespawnTimeFinished);
}
