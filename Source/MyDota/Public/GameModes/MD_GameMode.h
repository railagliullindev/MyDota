// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Widgets/InGame/MinimapWIdget.h"
#include "MD_GameMode.generated.h"

class AFogOfWarManager;
class AMD_GameState;
class AMD_CharacterBase;

UENUM()
enum class EMatchStage : uint8
{
	None,
	WaitingForPlayers,
	Draft,
	PrepareForBattle,
	PreGame,
	InProgress,
	PostGame
};

/**
 *
 */
UCLASS(Abstract, HideDropdown)
class MYDOTA_API AMD_GameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AMD_GameMode();

	void ProcessHeroSelection(const APlayerController* PC, const int32 RequestedHeroId);
	FVector GetBaseLocation(EMDTeam Team) const;

	void InitializePlayerData(const APlayerController* NewPC) const;

protected:

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void WaitingForPlayers();
	void Draft();
	void PrepareForBattle();
	void PreGame();
	void InProgress();
	void PostGame();

	void SpawnCameraForPlayer(APlayerController* NewPlayer);

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> CameraPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<AFogOfWarManager> FogManagerClass;

	UPROPERTY()
	AMD_GameState* GS;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector RadiantSpawnLocation;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector DireSpawnLocation;

private:

	UPROPERTY()
	AFogOfWarManager* RadiantFogManager;
	UPROPERTY()
	AFogOfWarManager* DireFogManager;

	EMatchStage MatchStage;

	void StartStageTimer(const float Duration);
	void UpdateStageTimer();

	void StartGameClock();

	FTimerHandle StageTimerHandle;

	void MoveToNextStage();

	const float WaitingForPlayersTime = 3.0f;
	const float DraftTime = 60.0f;
	const float PrepareForBattleTime = 5.f;
	const float PreGameTime = 30.0f;
	const float PostGameTime = 15.0f;

	int32 MatchTime;
};
