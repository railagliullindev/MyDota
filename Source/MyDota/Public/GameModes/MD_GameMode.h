// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MD_GameMode.generated.h"

class AMD_CharacterBase;
/**
 * 
 */
UCLASS()
class MYDOTA_API AMD_GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AMD_GameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void StartMatch() override;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> CameraPawnClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<AMD_CharacterBase> HeroClass;
	
private:
	bool bIsTeamA;
};
