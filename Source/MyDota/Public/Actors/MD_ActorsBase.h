// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/MDTeamInterface.h"
#include "MD_ActorsBase.generated.h"

UCLASS()
class MYDOTA_API AMD_ActorsBase : public AActor, public IMDTeamInterface
{
	GENERATED_BODY()

public:

	AMD_ActorsBase();

	virtual EMDTeam GetTeam() const override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	EMDTeam Team;
};
