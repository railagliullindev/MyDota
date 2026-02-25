// Rail Agliullin Dev. All Rights Reserved


#include "Controllers/MD_PlayerController.h"

#include "AIController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "Characters/MD_CharacterBase.h"
#include "Net/UnrealNetwork.h"

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

void AMD_PlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMD_PlayerController, Hero);
}

void AMD_PlayerController::SetHero(AMD_CharacterBase* InHero)
{
	if (HasAuthority())
	{
		Hero = InHero;
		
		OnRep_Hero();
	}
}

void AMD_PlayerController::OnRep_Hero()
{
	if (Hero && IsLocalController())
	{
		// Логика на клиенте: например, центрируем камеру на герое при спавне
		// Или обновляем HUD, связывая его с GAS атрибутами героя
		UE_LOG(LogTemp, Warning, TEXT("Герой реплицирован и готов к управлению!"));
	}
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
	
	if (HasAuthority())
	{
		SpawnHero();
	}
}

void AMD_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickMoveAction)
		{
			EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &AMD_PlayerController::InputMove);
		}
	}
}

void AMD_PlayerController::InputMove()
{
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		Server_MoveToLocation(Hit.ImpactPoint);
		
		// Визуал клика
	}
}

void AMD_PlayerController::Server_MoveToLocation_Implementation(FVector InLocation)
{
	if (Hero)
	{
		AAIController* AIC = Cast<AAIController>(Hero->GetController());
		if (AIC)
		{
			AIC->MoveToLocation(InLocation, 5.f);
		}
	}
}

void AMD_PlayerController::SpawnHero()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	Hero = GetWorld()->SpawnActor<AMD_CharacterBase>(AMD_CharacterBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}
