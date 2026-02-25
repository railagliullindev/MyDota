// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "MD_PlayerController.generated.h"

class AMD_CharacterBase;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class MYDOTA_API AMD_PlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	AMD_PlayerController();
	
	//~ Begin IGenericTeamAgentInterface Interface.
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~ End IGenericTeamAgentInterface Interface

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHero(AMD_CharacterBase* InHero);
	
	UFUNCTION()
	void OnRep_Hero();
	
	FORCEINLINE AMD_CharacterBase* GetHero() const {return Hero;}
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(ReplicatedUsing = OnRep_Hero)
	AMD_CharacterBase* Hero;
	
	void InputMove();
	
	UFUNCTION(Server, Reliable)
	void Server_MoveToLocation(FVector InLocation);
	
	void SpawnHero();

protected:
	/** Mapping context that contains click-to-move binding */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* ClickMoveMappingContext;

	/** Input action that is bound to mouse click for movement */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ClickMoveAction;

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
	
	FVector CachedDestination;
};
