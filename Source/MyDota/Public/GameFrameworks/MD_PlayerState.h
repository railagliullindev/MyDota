// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "MyDotaStructTypes.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/MDTeamInterface.h"
#include "MD_PlayerState.generated.h"

class UMD_AbilitySystemComponent;
class UMD_AttributeSet;
class AMD_CharacterBase;
/**
 *
 */
UCLASS()
class MYDOTA_API AMD_PlayerState : public APlayerState, public IAbilitySystemInterface, public IMDTeamInterface
{
	GENERATED_BODY()

public:

	AMD_PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UMD_AttributeSet* GetAttributeSet() const;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	TSubclassOf<AMD_CharacterBase> SelectedHeroClass;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSelectedHero(TSubclassOf<AMD_CharacterBase> InHeroClass);

	UPROPERTY(ReplicatedUsing = OnRep_Team, BlueprintReadOnly, Category = "Team")
	EMDTeam Team = EMDTeam::None;

	UFUNCTION()
	void OnRep_Team();

	virtual EMDTeam GetTeam() const override;

protected:

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	UMD_AbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	UMD_AttributeSet* AttributeSet;
};
