// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameModes/MD_GameMode.h"
#include "MD_GameState.generated.h"


class AMD_CharacterBase;
/**
 * 
 */
UCLASS()
class MYDOTA_API AMD_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	// Массив выбранных классов
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	TArray<TSubclassOf<AMD_CharacterBase>> PickedHeroClasses;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchStage, BlueprintReadOnly, Category = "MatchStage")
	EMathStage MathStage;
	
	void SetMatchStage(EMathStage NewStage);
	
	bool IsHeroAlreadyPicked(TSubclassOf<AMD_CharacterBase> InHeroClass) const;
	bool AreAllHeroesSelected() const;
	
	void MarkHeroAsPicked(TSubclassOf<AMD_CharacterBase> InHeroClass);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UFUNCTION()
	void OnRep_MatchStage();
};
