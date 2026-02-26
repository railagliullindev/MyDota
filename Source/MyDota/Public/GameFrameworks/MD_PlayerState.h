// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MD_PlayerState.generated.h"

class AMD_CharacterBase;
/**
 * 
 */
UCLASS()
class MYDOTA_API AMD_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	TSubclassOf<AMD_CharacterBase> SelectedHeroClass;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	bool bIsTeamA;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetSelectedHero(TSubclassOf<AMD_CharacterBase> InHeroClass);
};
