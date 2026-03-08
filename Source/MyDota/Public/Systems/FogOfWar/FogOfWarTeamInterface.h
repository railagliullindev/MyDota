// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FogOfWarTeamInterface.generated.h"

enum class EMDTeam : uint8;

UINTERFACE(MinimalAPI)
class UFogOfWarTeamInterface : public UInterface
{
	GENERATED_BODY()
};

class MYDOTA_API IFogOfWarTeamInterface
{
	GENERATED_BODY()

public:

	virtual EMDTeam GetTeam() const = 0;
};
