// Rail Agliullin Dev. All Rights Reserved


#include "Controllers/MD_PlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"

AMD_PlayerController::AMD_PlayerController()
{
	GenericTeamId = FGenericTeamId(0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

FGenericTeamId AMD_PlayerController::GetGenericTeamId() const
{
	return GenericTeamId;
}

void AMD_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (ClickMoveMappingContext)
			{
				Subsystem->AddMappingContext(ClickMoveMappingContext, /*Priority*/0);
			}
		}
	}
}

void AMD_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickMoveAction)
		{
			EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &AMD_PlayerController::HandleClickMove);
		}
	}
}

void AMD_PlayerController::HandleClickMove()
{	
	FHitResult HitResult;
	if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		return;
	}

	const FVector Destination = HitResult.ImpactPoint;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return;
	}
	
	FNavLocation ProjectedLocation;
	if (NavSys->ProjectPointToNavigation(Destination, ProjectedLocation))
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, ProjectedLocation.Location);
	}
}