// Rail Agliullin Dev. All Rights Reserved


#include "Controllers/MD_PlayerController.h"

AMD_PlayerController::AMD_PlayerController()
{
	GenericTeamId = FGenericTeamId(0);
}

FGenericTeamId AMD_PlayerController::GetGenericTeamId() const
{
	return GenericTeamId;
}