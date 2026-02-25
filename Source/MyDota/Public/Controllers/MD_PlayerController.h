// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "MD_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

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

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Called when click-to-move input is triggered */
	void HandleClickMove();

protected:
	/** Mapping context that contains click-to-move binding */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* ClickMoveMappingContext;

	/** Input action that is bound to mouse click for movement */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ClickMoveAction;
	
private:
	FGenericTeamId GenericTeamId;
};
