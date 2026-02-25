// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MD_GameMode.generated.h"

class AMD_CharacterBase;
/**
 * 
 */
UCLASS()
class MYDOTA_API AMD_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APawn> CameraPawnClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<AMD_CharacterBase> HeroClass;
};
