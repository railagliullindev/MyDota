// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MD_GameMode.generated.h"

class AMD_GameState;
class AMD_CharacterBase;


UENUM()
enum class EMathStage : uint8
{
	WaitingForPlayers,
	Draft,
	PreGame,
	InProgress,
	PostGame
};

/**
 * 
 */
UCLASS()
class MYDOTA_API AMD_GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AMD_GameMode();
	
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	void SetMatchStage(EMathStage NewStage);
	
protected:
	
	void WaitingForPlayers();
	void Draft();
	void PreGame();
	void InProgress();
	void PostGame();
	
	void SpawnCameraForPlayer(APlayerController* NewPlayer);
	
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> CameraPawnClass;
	
	EMathStage MatchStage;
	
	UPROPERTY()
	AMD_GameState* MD_GameState;
private:
	
	bool bIsTeamA;
};
