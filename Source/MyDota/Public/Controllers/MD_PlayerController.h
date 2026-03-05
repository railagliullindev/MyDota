// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "GameModes/MD_GameMode.h"
#include "Interfaces/MDTeamInterface.h"
#include "MD_PlayerController.generated.h"

class AMD_CharacterBase;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 *
 */
UCLASS()
class MYDOTA_API AMD_PlayerController : public APlayerController, public IGenericTeamAgentInterface, public IMDTeamInterface
{
	GENERATED_BODY()

public:

	AMD_PlayerController();

	//~ Begin IGenericTeamAgentInterface Interface.
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~ End IGenericTeamAgentInterface Interface

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual EMDTeam GetTeam() const override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Draft")
	void SelectHero(int32 InHeroID);

	void SetHero(AMD_CharacterBase* InHero);
	UFUNCTION()
	void OnRep_Hero();

	FORCEINLINE AMD_CharacterBase* GetHero() const
	{
		return Hero;
	}

	UFUNCTION(Client, Reliable, Category = "MatchStage")
	void SetMatchMode(EMathStage InMatchStage);

protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(ReplicatedUsing = OnRep_Hero)
	AMD_CharacterBase* Hero;

	void InputMove();
	void InputAttack();

	UFUNCTION(Server, Reliable)
	void Server_MoveToLocation(FVector InLocation);

	UFUNCTION(Server, Reliable)
	void Server_AttackTarget(AActor* Target);

	void SpawnHero();

protected:

	/** Mapping context that contains click-to-move binding */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* ClickMoveMappingContext;

	/** Input action that is bound to mouse click for movement */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ClickMoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* AttackAction;

	/** Input action that is bound to mouse click for movement */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FollowToHeroAction;

	/** Input action for camera movement */
	UPROPERTY(EditDefaultsOnly, Category = "Input|Camera")
	UInputAction* CameraMoveAction;

	/** Input action for camera zoom */
	UPROPERTY(EditDefaultsOnly, Category = "Input|Camera")
	UInputAction* CameraZoomAction;

	/** Класс героя, который будет заспавнен для этого игрока */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Setup")
	TSubclassOf<AMD_CharacterBase> HeroClass;

private:

	FGenericTeamId GenericTeamId;
	EMathStage MatchStage;
	FVector CachedDestination;
};
