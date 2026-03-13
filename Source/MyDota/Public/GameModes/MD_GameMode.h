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
	WaitingForPlayers,
	Draft,
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

	void InitializePlayerData(APlayerController* NewPC);

protected:

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void SetMatchStage(EMatchStage NewStage);
	void WaitingForPlayers();
	void Draft();
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

	// Координаты (можно задать прямо в BP-версии GameMode)
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

	// Храним прямые ссылки на PlayerController для быстрого доступа
	UPROPERTY()
	TArray<APlayerController*> RadiantPlayers;

	UPROPERTY()
	TArray<APlayerController*> DirePlayers;
};
