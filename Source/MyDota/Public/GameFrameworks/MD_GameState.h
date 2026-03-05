// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameModes/MD_GameMode.h"
#include "MD_GameState.generated.h"

struct FMatchHeroesInfo;
class AMD_CharacterBase;
/**
 *
 */
UCLASS()
class MYDOTA_API AMD_GameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Массив выбранных классов
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Draft")
	TArray<TSubclassOf<AMD_CharacterBase>> PickedHeroClasses;

	// Стадия матча
	UPROPERTY(ReplicatedUsing = OnRep_MatchStage, BlueprintReadOnly, Category = "MatchStage")
	EMathStage MathStage;

	UPROPERTY(Replicated)
	TArray<FMatchHeroesInfo> HeroesInfo;

	// Установка стадии матча
	void SetMatchStage(EMathStage NewStage);

	/** Draft  */
	bool AreAllHeroesSelected() const;

	bool IsHeroAlreadyPicked(const int32 HeroIndex) const;
	void RegisterHeroSelection(const int32 InPlayerId, const int32 HeroId);

	void RegisterNewPlayer(const int32 PlayerId, const int32 TeamId);

protected:

	UFUNCTION()
	void OnRep_MatchStage();
};
