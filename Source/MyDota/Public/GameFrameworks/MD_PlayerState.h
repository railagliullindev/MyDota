// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "MyDotaStructTypes.h"
#include "GameFramework/PlayerState.h"
#include "Systems/FogOfWar/FogOfWarTeamInterface.h"
#include "MD_PlayerState.generated.h"

// Делегат для UI (вызывается на всех клиентах при получении данных)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRespawnStatusChanged, bool, bIsDead, float, SecondsLeft);

class UMDHeroInfoDataAsset;
class UMD_AbilitySystemComponent;
class UMD_AttributeSet;
class AMD_CharacterBase;
/**
 *
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API AMD_PlayerState : public APlayerState, public IAbilitySystemInterface, public IFogOfWarTeamInterface
{
	GENERATED_BODY()

public:

	AMD_PlayerState();

	// ~ begin IAbilitySystemInterface Interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// ~ end IAbilitySystemInterface Interface

	// ~ begin IMDTeamInterface Interface
	virtual EMDTeam GetTeam() const override;
	// ~ end IMDTeamInterface Interface

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UMD_AttributeSet* GetAttributeSet() const;

	UFUNCTION()
	void OnRep_HeroId();

	UPROPERTY(ReplicatedUsing = OnRep_Team, BlueprintReadOnly)
	EMDTeam Team = EMDTeam::None;

	UPROPERTY(ReplicatedUsing = OnRep_HeroId, BlueprintReadOnly)
	int32 HeroId = -1;

	TSubclassOf<AMD_CharacterBase> SelectedHeroClass;

	// Метка времени сервера, когда герой должен ожить
	UPROPERTY(ReplicatedUsing = OnRep_RespawnTimeFinished, BlueprintReadOnly, Category = "Death")
	float RespawnTimeFinished = 0.0f;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRespawnStatusChanged OnRespawnStatusChanged;

	UFUNCTION()
	void OnRep_RespawnTimeFinished();

protected:

	UFUNCTION()
	void OnRep_Team();

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	UMD_AbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	UMD_AttributeSet* AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	UMDHeroInfoDataAsset* HeroInfoData;
};
