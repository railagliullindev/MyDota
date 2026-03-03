// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "MD_PlayerState.generated.h"

class UMD_AbilitySystemComponent;
class UMD_AttributeSet;
class AMD_CharacterBase;
/**
 *
 */
UCLASS()
class MYDOTA_API AMD_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AMD_PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UMD_AttributeSet* GetAttributeSet() const;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	TSubclassOf<AMD_CharacterBase> SelectedHeroClass;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	bool bIsTeamA;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSelectedHero(TSubclassOf<AMD_CharacterBase> InHeroClass);

protected:

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	UMD_AbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	UMD_AttributeSet* AttributeSet;
};
